'use strict';

const path = require('path');

const pkg = require('./package');

exports.binPath = path.join(__dirname, 'build', 'Release', pkg.name);
