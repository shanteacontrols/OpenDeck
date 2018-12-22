(function (app) {
    'use strict';

    app.factory('errors', function() {
        // p: [block, section, index]
        var errorCodes = {
            STATUS: 2,
            HANDSHAKE: 3,
            WISH:4,
            AMMOUNT:5,
            BLOCK:6,
            SECTION:7,
            PART: 8,
            INDEX: 9,
            NEW_VALUE: 10,
            MSG_LENGTH: 11,
            WRITE: 12,
            NOT_SUPPORTED: 13,
            READ: 14
        }

        function getErrorCode(value) {
            for( var prop in errorCodes) {
                if(errorCodes.hasOwnProperty(prop)) {
                    if(errorCodes[ prop ] === value)
                        return prop;
                }
            }
        }
        return {
            getErrorCode: getErrorCode
        }
    });
})(angular.module('app'));