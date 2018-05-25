(function (app) {
    'use strict';

    app.controller('midiCtrl', function($rootScope, $scope, repository, meta) {
        $scope.loading = true;
        repository.get(meta.midi).then(function(e) {
             $scope.midiObj = e;
             $scope.$watchCollection('midiObj', function(newValue, oldValue) {
                if (oldValue && newValue) {
                    for (var prop in $scope.midiObj) {
                        if(newValue[prop] !== oldValue[prop]) {
                            repository.set(meta.midi, prop, newValue[prop]);
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