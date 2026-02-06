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

app.get('/search', (req, res) => {
    const { q } = req.query;
    if (!q) return res.status(400).json({ error: 'Missing search query' });

    const threshold = 2;

    engine.search(q, threshold, (err, results) => {
        if (err) {
            console.error("Search Error:", err);
            return res.status(500).json({ error: 'Search failed', details: err.message });
        }
        res.json({ results });
    });
});

app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});
