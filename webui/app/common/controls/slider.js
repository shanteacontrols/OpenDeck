(function (app) {
    'use strict';

    app.directive("rSlider", function($timeout, $window) {
				return {
					restrict : "E",
					scope: { min: '=', max: '=', value: '=', tooltip: '@'},
					templateUrl : '/app/common/controls/slider.html',
					controller: function($scope) {   
						$scope.debounce = 600;   
						$scope.increment = function increment() {
							if ($scope.value < $scope.max)
								$scope.value++;
						};
						$scope.decrement = function decrement() {
							if ($scope.value > $scope.min)
								$scope.value--;
						};
					},
					link: function(scope, element, attributes) {
						angular.element(document).ready(function() { 
						    element.find('[data-toggle=tooltip]').tooltip({trigger: 'hover'});
							// $('[data-toggle=tooltip]').tooltip({ trigger: "hover" });
						});
					}
				}
			});

})(angular.module('app'));