const path = require('path');

// Try loading Release, fallback to Debug
let searchAddon;
try {
    searchAddon = require('./build/Release/search');
} catch (e) {
    try {
        searchAddon = require('./build/Debug/search');
    } catch (e2) {
        console.error("Could not load search addon.");
        process.exit(1);
    }
}

const { SearchEngine } = searchAddon;

const engine = new SearchEngine(path.join(__dirname, 'sample.txt'));
const query = process.argv[2] || 'hello';

console.log(`Searching for: ${query}`);
console.time("Search");
engine.search(query, 2, (err, results) => {
    console.timeEnd("Search");
    if (err) {
        console.error("Error:", err);
    } else {
        console.log("Results:", results);
    }
});
