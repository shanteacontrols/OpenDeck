(function (app) {
    'use strict';

    app.controller('ledsCtrl', function ($scope, repository, meta, $uibModal) {
        $scope.objects = [];
        $scope.loading = true;

        repository.getCollection(9).then(function (d) {
            $scope.objects = d;
            var _meta = angular.copy(meta.ledGlobal);
            repository.get(_meta).then(function (d) {
                $scope.obj = d;
                $scope.$watchCollection('obj', function (newValue, oldValue) {
                    if (oldValue && newValue) {
                        for (var prop in $scope.obj) {
                            if (newValue[prop] !== oldValue[prop]) {
                                repository.set(_meta, prop, newValue[prop]);
                            }
                        }
                    }
                });
                $scope.loading = false;
                if (!$scope.$$phase) {
                    $scope.$apply();
                }
            });
        });

        $scope.openModal = openModal;

        function openModal(idx, obj) {
            $scope.idx = idx;
            //  $scope.obj = obj;
            $uibModal.open({
                templateUrl: 'webui/app/leds/led.html',
                size: 'lg',
                controller: 'ledCtrl',
                scope: $scope
            });
        }
    });

})(angular.module('app'));