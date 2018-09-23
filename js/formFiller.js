(function(window)
 {
     function fillPassword(loginInput, passInput)
     {
         if (window.pwManager)
         {
             window.pwManager.getCredentials(
                 window.location.host, window.location.pathname,
                 function(retVal) {
                     loginInput.value = retVal.login
                     if (typeof angular !== 'undefined') {
                         angular.element(loginInput).triggerHandler('input')
                     }
                     passInput.value = retVal.pass
                     if (typeof angular !== 'undefined') {
                         e = angular.element(passInput).triggerHandler('input')
                     }
                 });
         } else {
             console.log("formFiller: Couldn't find pwManager");
         }
     }

     for (var i = 0; i < document.forms.length; ++i)
     {
         var form = document.forms[i];
         var inputs = form.querySelectorAll('input:not([disabled])[name]');

         var login = false;
         var loginInput;
         var passInput;
         var pass = false;

         for (var j = 0; j < inputs.length; ++j)
         {
             if (inputs[j].type == 'email' || inputs[j].type == 'text')
             {
                 loginInput = inputs[j]
                 login = true;
             }
             if (inputs[j].type == 'password')
             {
                 passInput = inputs[j]
                 pass = true;
             }
         }

         if (login && pass)
         {
             if (!window.pwManager) {
                 window.channel = new QWebChannel(
                     qt.webChannelTransport,
                     function(channel) {
                         window.pwManager = channel.objects.pwManager
                         fillPassword(loginInput, passInput)
                     });
             }
             else {
                 fillPassword(loginInput, passInput);
             }
         }
     }
 }
)(window);
