'use strict';

const devicelog = require('..');
const _ = require('macaca-utils');

describe('test', function() {
  it('should be ok', function() {
    const isExisted = _.isExistedFile(devicelog.binPath);
    isExisted.should.be.true();
  });
});
