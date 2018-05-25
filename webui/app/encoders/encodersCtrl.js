(function (app) {
    'use strict';

     app.controller('encodersCtrl', function($scope, repository, $uibModal) {
        $scope.openModal = openModal;
        function openModal(idx) {
             $scope.idx = idx;
             $uibModal.open({ templateUrl: '/app/encoders/encoder.html', size: 'lg', controller: 'encoderCtrl', scope: $scope });
        }
        $scope.objects = [];
        repository.getCollection(7).then(function(d) {
            $scope.objects = d;
        });
	});
	
})(angular.module('app'));