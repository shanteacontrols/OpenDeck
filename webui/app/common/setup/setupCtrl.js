(function (app) {
    'use strict';
    app.controller('setupCtrl', function($scope, $uibModal, repository, system) {
        $scope.backup = backup;
        $scope.restore = restore;
        $scope.openModal = openModal;
        function openModal(idx) {
             $uibModal.open({ templateUrl: 'webui/app/common/setup/info.html', size: 'lg', controller: 'infoCtrl', scope: $scope });
        }
        function backup() {
            repository.backup().then(function (d) {
                var file = new File([JSON.stringify(d)], "openDeckBackup.txt",
                 {type: "text/plain;charset=utf-8"});
                saveAs(file);
            });

        }
        function restore(files) {
            if (files.length < 1) return;
            var file = files[0];
            var reader = new FileReader();
            reader.readAsText(file);
            reader.onload = function(progressEvent){
                var data = JSON.parse(this.result);
                repository.restore(data).then(function() {
                    system.reboot();
                });
        }
        }
    });

})(angular.module('app'));