(function (app) {
    'use strict';

    app.controller('infoCtrl', function ($scope, $rootScope, api, system, $http, $uibModalInstance, $confirm, activity) {

        $scope.factoryReset = factoryReset;
        $scope.reboot = reboot;
        $scope.bootloaderMode = bootloaderMode;
        $scope.checkUpdate = checkUpdate;
        startLoading();

        system.version().then(function (d) {
            $scope.firmwareVersion = 'v' + d[6] + '.' + d[7] + '.' + d[8];

            //determine board variant
            if ((d[9] == 1) && (d[10] == 91) && (d[11] == 42) && (d[12] == 85))
                $scope.boardName = 'OpenDeck';
            else if ((d[9] == 1) && (d[10] == 52) && (d[11] == 50) && (d[12] == 124))
                $scope.boardName = 'Arduino Leonardo';
            else if ((d[9] == 1) && (d[10] == 92) && (d[11] == 110) && (d[12] == 58))
                $scope.boardName = 'Arduino Mega';
            else if ((d[9] == 1) && (d[10] == 46) && (d[11] == 56) && (d[12] == 29))
                $scope.boardName = 'Arduino Pro Micro';
            else if ((d[9] == 1) && (d[10] == 11) && (d[11] == 120) && (d[12] == 50))
                $scope.boardName = 'Arduino Uno';
            else if ((d[9] == 1) && (d[10] == 100) && (d[11] == 8) && (d[12] == 4))
                $scope.boardName = 'Teensy++ 2.0';
            else if ((d[9] == 1) && (d[10] == 5) && (d[11] == 75) && (d[12] == 68))
                $scope.boardName = 'DubFocus';
            else if ((d[9] == 1) && (d[10] == 44) && (d[11] == 68) && (d[12] == 62))
                $scope.boardName = 'Bergamot';
            else if ((d[9] == 1) && (d[10] == 109) && (d[11] == 54) && (d[12] == 20))
                $scope.boardName = 'STM32F4 Discovery';
            else if ((d[9] == 1) && (d[10] == 78) && (d[11] == 126) && (d[12] == 38))
                $scope.boardName = 'Jamiel';
            else if ((d[9] == 1) && (d[10] == 69) && (d[11] == 56) && (d[12] == 9))
                $scope.boardName = 'Jose';
            else if ((d[9] == 1) && (d[10] == 73) && (d[11] == 70) && (d[12] == 126))
                $scope.boardName = 'Arrival';
            else
                $scope.boardName = 'UNKNOWN BOARD';

            $scope.hardwareVersion = 'v' + d[13] + '.' + d[14] + '.x';
            finishLoading();
        });

        function reboot() {
            $uibModalInstance.dismiss('cancel');
            startLoading();
            system.reboot();
        }

        function factoryReset() {
            $uibModalInstance.dismiss('cancel');
            startLoading();
            system.factoryReset();
        }

        function bootloaderMode() {
            $uibModalInstance.dismiss('cancel');
            startLoading();
            system.bootloaderMode();
        }

        function checkUpdate() {
            var url = "https://api.github.com/repos/paradajz/OpenDeck/tags";
            $http.get(url).then(function (d) {
                var data = d.data;

                function compare(a, b) {
                    var x = strip(a.name);
                    var y = strip(b.name);
                    var a_ver = x.toString().split('.');
                    var b_ver = y.toString().split('.');

                    for (var i = 0; i < a_ver.length; ++i) {
                        a_ver[i] = Number(a_ver[i]);
                    }
                    for (var i = 0; i < b_ver.length; ++i) {
                        b_ver[i] = Number(b_ver[i]);
                    }
                    if (a_ver.length == 2) {
                        a_ver[2] = 0;
                    }

                    if (a_ver[0] > b_ver[0]) return true;
                    if (a_ver[0] < b_ver[0]) return false;

                    if (a_ver[1] > b_ver[1]) return true;
                    if (a_ver[1] < b[1]) return false;

                    if (a_ver[2] > b_ver[2]) return true;
                    if (a_ver[2] < b_ver[2]) return false;

                    return true;
                }

                var maxObj = data.sort(compare)[data.length - 1];
                var max = maxObj.name.replace(/\D/g, '');
                var current = $scope.firmwareVersion.replace(/\D/g, '');
                //console.log('Latest version: ' + max + '\n' + 'Current version: ' + current);
                if (max > current) {
                    $confirm({
                            text: 'New firmware version is available. Do you wish to proceed with download?'
                        })
                        .then(function () {
                            window.location.assign(maxObj.zipball_url);
                        });
                } else {
                    $confirm({
                        text: 'Your firmware version is up to date.'
                    });
                }
            });
        }

        function strip(target) {
            return new Number(target.replace(/v/g, ''));
        }

        function startLoading() {
            $scope.loading = true;
            $rootScope.loading = true;
        }

        function finishLoading() {
            $scope.loading = false;
            $rootScope.loading = false;
        }
    });

})(angular.module('app'));