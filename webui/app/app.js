(function () {
    'use strict';

    var app = angular.module('app', ['ngRoute', 'ui.bootstrap']);

    app.config(config);
    app.run(run);
        
    function config($routeProvider, $locationProvider) {
        $locationProvider
                .html5Mode({ enabled: false, requireBase: false })
                .hashPrefix('');
        $routeProvider
            .when("/", {
                templateUrl: "/OpenDeck/webui/app/midi/midi.html",
                controller: "midiCtrl"
            })
            .when("/midi/", {
                templateUrl: "/OpenDeck/webui/app/midi/midi.html",
                controller: "midiCtrl"
            })
            .when("/buttons/", {
                templateUrl: "/OpenDeck/webui/app/buttons/buttons.html",
                controller: "buttonsCtrl"
            })
            .when("/leds/", {
                templateUrl: "/OpenDeck/webui/app/leds/leds.html",
                controller: "ledsCtrl"
            })
            .when("/encoders/", {
                templateUrl: "/OpenDeck/webui/app/encoders/encoders.html",
                controller: "encodersCtrl"
            })
            .when("/analog/", {
                templateUrl: "/OpenDeck/webui/app/analog/analogs.html",
                controller: "analogsCtrl"
            })
            .when("/display/", {
                templateUrl: "/OpenDeck/webui/app/display/display.html",
                controller: "displayCtrl"
            })
            .when("/test/", {
                templateUrl: "/OpenDeck/webui/app/test/test.html",
                controller: "testCtrl"
            })
            .otherwise({ redirectTo: "/" });
    };
   function run($rootScope, system) {
        $rootScope.loading = true;
        system.connect().then(function() {
			
			$rootScope.$on('connected', function() {
				system.handshake().then(function(d) {
					$rootScope.loading = false;
					safeApply();
				});
			});
			$rootScope.$on('disconnected', function() {
					 $rootScope.loading = true;
					safeApply();
			});
			
			$rootScope.loading = false;
			safeApply();
        });

        var safeApply = function() {
           if (!$rootScope.$$phase) 
                 $rootScope.$apply();
        }
   }
})();