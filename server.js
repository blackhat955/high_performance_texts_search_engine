const express = require('express');
const path = require('path');

// Try loading Release, fallback to Debug
let searchAddon;
try {
    searchAddon = require('./build/Release/search');
} catch (e) {
    try {
        searchAddon = require('./build/Debug/search');
    } catch (e2) {
        console.error("Could not load search addon. Make sure to run 'npm run build'");
        process.exit(1);
    }
}

const { SearchEngine } = searchAddon;

const app = express();
const PORT = process.env.PORT || 3000;

app.use(express.static(path.join(__dirname, 'public')));

// Initialize Engine once
console.log("Loading Search Engine...");
const engine = new SearchEngine(path.join(__dirname, 'sample.txt'));
console.log("Search Engine Loaded.");

// Performance Tracking
let lastSearchDuration = 0;
let lastMultiThreadDuration = 0;
let totalSearches = 0;
let loadedMode = 'Unknown';

// Determine mode safely
try {
    const releasePath = './build/Release/search';
    // Only compare if we can load it
    try {
        if (searchAddon === require(releasePath)) loadedMode = 'Release';
    } catch {}
} catch {}

try {
    const debugPath = './build/Debug/search';
    try {
        if (searchAddon === require(debugPath)) loadedMode = 'Debug';
    } catch {}
} catch {}

app.get('/status', (req, res) => {
    const mem = process.memoryUsage();
    res.json({
        rss: (mem.rss / 1024 / 1024).toFixed(2), // MB
        heapTotal: (mem.heapTotal / 1024 / 1024).toFixed(2),
        heapUsed: (mem.heapUsed / 1024 / 1024).toFixed(2),
        uptime: process.uptime().toFixed(0),
        mode: loadedMode,
        totalSearches,
        lastSearchDuration,
        lastMultiThreadDuration
    });
});

app.get('/search', (req, res) => {
    const { q } = req.query;
    if (!q) return res.status(400).json({ error: 'Missing search query' });

    const threshold = 2;

    // Single Thread Measurement
    const start = process.hrtime();
    engine.search(q, threshold, (err, results) => {
        const end = process.hrtime(start);
        lastSearchDuration = (end[0] * 1000 + end[1] / 1e6).toFixed(2); // ms
        totalSearches++;

        if (err) {
            console.error("Search Error:", err);
            return res.status(500).json({ error: 'Search failed', details: err.message });
        }
        res.json({ results });
    });
});

app.get('/benchmark', async (req, res) => {
    const { q } = req.query;
    if (!q) return res.status(400).json({ error: 'Missing search query' });
    
    // Simulate Multi-thread load
    const CONCURRENCY = 10;
    const start = process.hrtime();
    
    const promises = [];
    for(let i=0; i<CONCURRENCY; i++) {
        promises.push(new Promise((resolve) => {
            engine.search(q, 2, (err, res) => resolve());
        }));
    }
    
    await Promise.all(promises);
    const end = process.hrtime(start);
    // Average time per request under load
    lastMultiThreadDuration = ((end[0] * 1000 + end[1] / 1e6) / CONCURRENCY).toFixed(2);
    
    res.json({ success: true, latency: lastMultiThreadDuration });
});


app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});
