(function (app) {
    'use strict';


    app.directive('items', function() {
         return {
            restrict: 'E',
            replace: true,
            templateUrl: '/OpenDeck/webui/app/common/menu/items.html',
           link: function(scope, el, attrs) {
                 el.find('ul li a').click(function() {
                    if ($(this).hasClass('disabled')) return false;
                    el.find('ul li').removeClass('active');
                    el.find('ul li a').removeClass('active');
                    el.find('ul li a').removeClass('disabled');
                    $(this).addClass('active');
                    $(this).addClass('disabled');
                    $(this).parent().addClass('active');
                })
            }

    }});
	
    app.directive('menu', function() {
         return {
            restrict: 'E',
            replace: true,
            templateUrl: '/OpenDeck/webui/app/common/menu/menu.html'

    }});

})(angular.module('app'));