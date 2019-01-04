(function (app) {
    'use strict';

    app.controller('displayCtrl', function ($rootScope, $scope, repository, meta) {
        $scope.loading = true;
        repository.get(meta.display).then(function (e) {
            $scope.displayObj = e;
            $scope.$watchCollection('displayObj', function (newValue, oldValue) {
                if (oldValue && newValue) {
                    for (var prop in $scope.displayObj) {
                        if (newValue[prop] !== oldValue[prop]) {
                            repository.set(meta.display, prop, newValue[prop]);
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

})(angular.module('app'));