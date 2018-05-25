'use strict';

var angular = require('angular');
var defaultPopoverTemplate = require('./angular-bootstrap-confirm.html');
require('angular-sanitize');
require('./ui-bootstrap-position');
var DEFAULT_POPOVER_URL = 'angular-bootstrap-confirm.html';

module.exports = angular

  .module('mwl.confirm', [
    'ngSanitize',
    'ui.bootstrap.position'
  ])

  .run(function($templateCache) {
    $templateCache.put(DEFAULT_POPOVER_URL, defaultPopoverTemplate);
  })

  .controller('PopoverConfirmCtrl', function($scope, $rootScope, $element, $attrs, $compile, $document, $window, $timeout,
                                             $injector, $templateRequest, $parse, $log, $animate, confirmationPopoverDefaults) {
    var vm = this;
    vm.defaults = confirmationPopoverDefaults;
    vm.$attrs = $attrs;
    var positionServiceName = $injector.has('$uibPosition') ? '$uibPosition' : '$position';
    var positionService = $injector.get(positionServiceName);
    var templateUrl = $attrs.templateUrl || confirmationPopoverDefaults.templateUrl;
    var popoverScope = $rootScope.$new(true);
    var animation = vm.animation = $attrs.animation === 'true' || confirmationPopoverDefaults.animation;
    popoverScope.vm = vm;

    function assignOuterScopeValue(attributeName, value) {
      var scopeName = $attrs[attributeName];
      if (angular.isDefined(scopeName)) {
        if ($parse(scopeName).assign) {
          $parse(scopeName).assign($scope, value);
        } else {
          $log.warn('Could not set value of ' + attributeName + ' to ' + value + '. This is normally because the value is not a variable.');
        }
      }
    }

    function evaluateOuterScopeValue(scopeName, defaultValue, locals) {
      if (angular.isDefined(scopeName)) {
        return $parse(scopeName)($scope, locals);
      } else {
        return defaultValue;
      }
    }

    var popoverLoaded = $templateRequest(templateUrl).then(function(template) {
      var popover = angular.element(template);
      popover.css('display', 'none');
      $compile(popover)(popoverScope);
      $document.find('body').append(popover);
      return popover;
    });

    vm.isVisible = false;

    function positionPopover() {
      popoverLoaded.then(function(popover) {
        var position = positionService.positionElements($element, popover, $attrs.placement || vm.defaults.placement, true);
        position.top += 'px';
        position.left += 'px';
        popover.css(position);
      });
    }

    function applyFocus() {
      var buttonToFocus = $attrs.focusButton || vm.defaults.focusButton;
      if (buttonToFocus) {
        popoverLoaded.then(function(popover) {
          var targetButtonClass = buttonToFocus + '-button';
          popover[0].getElementsByClassName(targetButtonClass)[0].focus();
        });
      }
    }

    function showPopover() {
      if (!vm.isVisible && !evaluateOuterScopeValue($attrs.isDisabled, false)) {
        popoverLoaded.then(function(popover) {
          popover.css({display: 'block'});
          if (animation) {
            $animate.addClass(popover, 'in');
          }
          positionPopover();
          applyFocus();
          vm.isVisible = true;
          assignOuterScopeValue('isOpen', true);
        });
      }
    }

    function hidePopover() {
      if (vm.isVisible) {
        popoverLoaded.then(function(popover) {
          if (animation) {
            $animate.removeClass(popover, 'in');
          }
          popover.css({display: 'none'});
          vm.isVisible = false;
          assignOuterScopeValue('isOpen', false);
        });
      }
    }

    function togglePopover() {
      if (!vm.isVisible) {
        showPopover();
      } else {
        hidePopover();
      }
    }

    function documentClick(event) {
      popoverLoaded.then(function(popover) {
        if (vm.isVisible && !popover[0].contains(event.target) && !$element[0].contains(event.target)) {
          hidePopover();
        }
      });
    }

    vm.showPopover = showPopover;
    vm.hidePopover = hidePopover;
    vm.togglePopover = togglePopover;

    vm.onConfirm = function(callbackLocals) {
      evaluateOuterScopeValue($attrs.onConfirm, null, callbackLocals);
    };

    vm.onCancel = function(callbackLocals) {
      evaluateOuterScopeValue($attrs.onCancel, null, callbackLocals);
    };

    $scope.$watch($attrs.isOpen, function(newIsOpenValue) {
      $timeout(function() { //timeout required so that documentClick() event doesn't fire and close it
        if (newIsOpenValue) {
          showPopover();
        } else {
          hidePopover();
        }
      });
    });

    $element.bind('click', togglePopover);

    $window.addEventListener('resize', positionPopover);

    $document.bind('click', documentClick);
    $document.bind('touchend', documentClick);

    $scope.$on('$destroy', function() {
      popoverLoaded.then(function(popover) {
        popover.remove();
        $element.unbind('click', togglePopover);
        $window.removeEventListener('resize', positionPopover);
        $document.unbind('click', documentClick);
        $document.unbind('touchend', documentClick);
        popoverScope.$destroy();
      });
    });

  })

  .directive('mwlConfirm', function() {

    return {
      restrict: 'A',
      controller: 'PopoverConfirmCtrl'
    };

  })

  .value('confirmationPopoverDefaults', {
    confirmText: 'Confirm',
    cancelText: 'Cancel',
    confirmButtonType: 'success',
    cancelButtonType: 'default',
    placement: 'top',
    focusButton: null,
    templateUrl: DEFAULT_POPOVER_URL,
    hideConfirmButton: false,
    hideCancelButton: false,
    animation: false
  })

  .name;
