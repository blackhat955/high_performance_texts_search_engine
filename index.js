const searchAddon = require('./build/Release/search');

const results = searchAddon.search('hello');
console.log('Search Results:', results);
