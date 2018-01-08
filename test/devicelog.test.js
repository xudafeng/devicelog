'use strict';

const devicelog = require('..');
const assert = require('assert');
const _ = require('macaca-utils');

describe('test', function() {
  it('should be ok', function() {
    const isExisted = _.isExistedFile(devicelog.binPath);
    assert(isExisted, true);
  });
});
