const express = require('express');
const path = require('path');
const addon = require('./build/Release/search');

const app = express();
const PORT = 3000;

app.use(express.static(path.join(__dirname, 'public')));


app.get('/search', (req, res) => {
    const { q, mode } = req.query;
    if (!q) return res.status(400).json({ error: 'Missing search query' });

    const threshold = 2;
    const isMultiThreaded = mode === 'multi';

    try {
        const results = addon.search(q, threshold, isMultiThreaded);
        res.json({ results });
    } catch (err) {
        res.status(500).json({ error: 'Search failed', details: err.message });
    }
});

app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});
