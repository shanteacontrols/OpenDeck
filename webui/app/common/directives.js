(function (app) {
    'use strict';

    app.directive('convertToNumber', function() {
        return {
            require: 'ngModel',
            link: function(scope, element, attrs, ngModel) {
            ngModel.$parsers.push(function(val) {
                return val == null ? null : parseInt(val, 10);
            });
            ngModel.$formatters.push(function(val) {
                return val == null ? null : '' + val ;
            });
            }
        };
    });
    

    app.directive('blinkOnAction', function($timeout, $rootScope) {
        return {
            scope: { block: '=', idx: '@' },
            link: function(scope, element, attrs) {
                scope.$on('sysex.componentInfo', function(event, args) {
                    if (attrs.idx == args.idx && attrs.block == args.block) {
                        element.addClass('enabled');
                        $timeout(function(d){
                            element.removeClass('enabled');
                        }, 300);
                    }
                })
            }
        }
    });
	
	app.directive('fileUpload', function() {
	  return {
		restrict: 'A',
		link: function (scope, element, attrs) {
		  var onChangeFunc = scope.$eval(attrs.fileUpload);
		  element.bind('change', onChangeFunc);
		}
	  };
	});


    app.directive('setup', function() {
        return {
            scope: {},
            templateUrl: '/OpenDeck/webui/app/common/setup/setup.html',
            link: function(scope, element, attrs) {

            }
        }
    });

    app.controller('confirmModalController', function($scope, $uibModalInstance, data) {
        $scope.data = angular.copy(data);

        $scope.ok = function() {
           $uibModalInstance.close();
        };

        $scope.cancel = function() {
           $uibModalInstance.dismiss('cancel');
        };
    })
    app.value('$confirmModalDefaults', {
        template: '<div class="modal-header"><h3 class="modal-title">Confirm</h3></div><div class="modal-body">{{data.text}}</div><div class="modal-footer"><button class="btn btn-primary" ng-click="ok()">OK</button><button class="btn btn-warning" ng-click="cancel()">Cancel</button></div>',
        controller: 'confirmModalController'
    })
    app.factory('$confirm', function($uibModal, $confirmModalDefaults) {
        return function(data, settings) {
            settings = angular.extend($confirmModalDefaults, (settings || {}));
            data = data || {};

            if ('templateUrl' in settings && 'template' in settings) {
                delete settings.template;
            }

            settings.resolve = { data: function() { return data; } };

            return $uibModal.open(settings).result;
        };
    })
    app.directive('confirm', function($confirm) {
        return {
            priority: 1,
            restrict: 'A',
            scope: {
                confirmIf: "=",
                ngClick: '&',
                confirm: '@'
            },
            link: function(scope, element, attrs) {
                function reBind(func) {
                    element.unbind("click").bind("click", function() {
                        func();
                    });
                }

                function bindConfirm() {
                    $confirm({ text: scope.confirm }).then(scope.ngClick);
                }

                if ('confirmIf' in attrs && attrs.confirmIf) {
                    scope.$watch('confirmIf', function(newVal) {
                        if (newVal) {
                            reBind(bindConfirm);
                        } else {
                            reBind(function() {
                                scope.$apply(scope.ngClick);
                            });
                        }
                    });
                } else {
                    reBind(bindConfirm);
                }
            }
        }
    });


})(angular.module('app'));