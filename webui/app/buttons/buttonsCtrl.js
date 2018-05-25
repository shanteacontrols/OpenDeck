(function (app) {
    'use strict';

    app.controller('buttonsCtrl', function($scope, repository, $uibModal, $timeout) {
        $scope.openModal = openModal;
        function openModal(idx) {
             $scope.idx = idx;
             $uibModal.open({ templateUrl: '/OpenDeck/webui/app/buttons/button.html', size: 'lg', controller: 'buttonCtrl', scope: $scope });
        }
        $scope.objects = [];
        repository.getCollection(6).then(function(d) {
            $scope.objects = d;
        });

	});
	
})(angular.module('app'));