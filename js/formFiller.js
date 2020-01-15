(function(window)
 {
     function fillPassword(loginInput, passInput)
     {
         if (typeof window.pwManager !== 'undefined')
         {
             window.pwManager.getCredentials(
                 channel.objects.dbName.objectName,
                 window.location.host, window.location.pathname,
                 function(retVal) {
                     loginInput.value = retVal.login
                     loginInput.dispatchEvent(new Event("input", { bubbles: true}));

                     passInput.value = retVal.pass
                     passInput.dispatchEvent(new Event("input", { bubbles: true}));
                     /*
                     if (typeof angular !== 'undefined') {
                         e = angular.element(passInput).triggerHandler('input')
                     }*/
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
             if (typeof window.pwManager == 'undefined' ) {
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
