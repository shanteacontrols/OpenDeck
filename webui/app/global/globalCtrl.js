(function (app) {
    'use strict';

    app.controller('globalCtrl', function($rootScope, $scope, repository, meta) {
        $scope.loading = true;
        repository.get(meta.global).then(function(e) {
             $scope.globalObj = e;
             $scope.$watchCollection('globalObj', function(newValue, oldValue) {
                if (oldValue && newValue) {
                    for (var prop in $scope.globalObj) {
                        if(newValue[prop] !== oldValue[prop]) {
                            repository.set(meta.global, prop, newValue[prop]);
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