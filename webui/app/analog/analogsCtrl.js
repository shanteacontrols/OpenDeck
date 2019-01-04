(function (app) {
    'use strict';

    app.controller('analogsCtrl', function ($scope, repository, $uibModal) {
        $scope.openModal = openModal;

        function openModal(idx) {
            $scope.idx = idx;
            $uibModal.open({
                templateUrl: 'webui/app/analog/analog.html',
                size: 'lg',
                controller: 'analogCtrl',
                scope: $scope
            });
        }
        $scope.objects = [];
        repository.getCollection(8).then(function (d) {
            $scope.objects = d;
        });
    });

})(angular.module('app'));