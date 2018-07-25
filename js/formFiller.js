(function(window)
 {
     function fillPassword(loginInput, passInput)
     {
         if (window.pwManager)
         {
             window.pwManager.getCredentials(
                 window.location.host, window.location.path,
                 function(retVal) {
                     loginInput.value = retVal.login
                     passInput.value = retVal.pass
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
         {// FIXME: make sure this gets closed when leaving page

             if (!window.pwManager) {
             var socket = new WebSocket("ws://localhost:61581")
             socket.onclose = function()
             {
                 console.error("formFiller: filler web channel closed");
             };
             socket.onerror = function(error)
             {
                 console.error("formFiler: filler web channel error: " + error);
             };
             socket.onopen = function()
             {
                 console.log("formFiller: filler socket opened")
                 window.channel = new QWebChannel(socket, function(channel)
                 {
                     window.pwManager = channel.objects.pwManager
                     fillPassword(loginInput, passInput);
                 });
             }
             }
         }
     }
 }
)(window);
