const { spawn, exec } = require('child_process');
const http = require('http');
const path = require('path');

// Configuration
const SERVER_PORT = 3000;
const MEMORY_THRESHOLD_MB = 200; // Warning threshold
const TOTAL_REQUESTS = 500;
const CONCURRENCY = 50;

// Path to server
const serverPath = path.join(__dirname, '..', 'server.js');

console.log("--- High Performance Search Engine Monitor ---");
console.log(`Target: ${serverPath}`);

// Spawn Server
// We inherit stdio for stderr to see errors, but pipe stdout to detect startup
const server = spawn(process.execPath, [serverPath], {
    cwd: path.join(__dirname, '..'),
    env: { ...process.env }, // Inherit environment (crucial for ASan/Debug vars)
    stdio: ['ignore', 'pipe', 'pipe']
});

let serverReady = false;
let monitorInterval;

server.stdout.on('data', (data) => {
    const msg = data.toString();
    // process.stdout.write(`[SERVER OUT] ${msg}`); // Optional: echo server output
    if (msg.includes(`Server running at http://localhost:${SERVER_PORT}`)) {
        if (!serverReady) {
            serverReady = true;
            console.log("Server is ready. Starting monitoring...");
            startMonitoring();
            startBenchmark();
        }
    }
});

server.stderr.on('data', (data) => {
    process.stderr.write(`[SERVER ERR] ${data}`);
});

server.on('close', (code) => {
    console.log(`Server process exited with code ${code}`);
    clearInterval(monitorInterval);
    process.exit(code || 0);
});

function startMonitoring() {
    monitorInterval = setInterval(() => {
        if (!server.pid) return;
        
        // Use ps to get RSS memory of the server process
        exec(`ps -o rss= -p ${server.pid}`, (err, stdout) => {
            if (!err && stdout) {
                const rssKB = parseInt(stdout.trim());
                if (!isNaN(rssKB)) {
                    const rssMB = (rssKB / 1024).toFixed(1);
                    const status = rssMB > MEMORY_THRESHOLD_MB ? '\x1b[31mSURGE\x1b[0m' : '\x1b[32mNORMAL\x1b[0m';
                    
                    process.stdout.write(`\r[MONITOR] RAM: ${rssMB} MB | Status: ${status} | PID: ${server.pid}  `);
                }
            }
        });
    }, 500);
}

async function startBenchmark() {
    // Give server a moment to settle
    await new Promise(r => setTimeout(r, 1000));
    
    console.log('\n\n--- Starting Benchmark ---');
    
    // Phase 1: Single Threaded
    console.log(`\n[Phase 1] Single-Threaded Sequential (${TOTAL_REQUESTS} reqs)`);
    const startSingle = Date.now();
    for (let i = 0; i < TOTAL_REQUESTS; i++) {
        await makeRequest('apple');
        if (i % 50 === 0) process.stdout.write('.');
    }
    const durSingle = Date.now() - startSingle;
    console.log(`\nDone. Time: ${durSingle}ms | Avg: ${(durSingle/TOTAL_REQUESTS).toFixed(2)}ms/req`);

    // Phase 2: Multi Threaded
    console.log(`\n[Phase 2] Multi-Threaded Concurrent (${TOTAL_REQUESTS} reqs, batch size ${CONCURRENCY})`);
    const startMulti = Date.now();
    
    // We do batches to control concurrency slightly, or just all at once?
    // Let's do batches to be realistic
    for (let i = 0; i < TOTAL_REQUESTS; i += CONCURRENCY) {
        const batch = [];
        for (let j = 0; j < CONCURRENCY && (i + j) < TOTAL_REQUESTS; j++) {
            batch.push(makeRequest('banana'));
        }
        await Promise.all(batch);
        process.stdout.write('#');
    }
    
    const durMulti = Date.now() - startMulti;
    console.log(`\nDone. Time: ${durMulti}ms | Throughput: ${(TOTAL_REQUESTS / (durMulti/1000)).toFixed(2)} req/s`);
    
    console.log('\n--- Benchmark Finished ---');
    console.log('Use Ctrl+C to stop, or wait 5s for auto-shutdown...');
    
    setTimeout(() => {
        server.kill();
        process.exit(0);
    }, 5000);
}

function makeRequest(query) {
    return new Promise((resolve, reject) => {
        const req = http.get(`http://localhost:${SERVER_PORT}/search?q=${query}`, (res) => {
            res.resume(); // consume data
            res.on('end', resolve);
        });
        req.on('error', (e) => {
            // Ignore connection errors during shutdown
            resolve();
        });
    });
}
