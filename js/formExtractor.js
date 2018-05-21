(function(window)
{
    function attachFormListener(form)
    {
        form.addEventListener('submit', function(event)
        {
            var inputs = form.querySelectorAll('input:not([disabled])[name]');
            var fields = [];
            var hasPassword = false;

            for (var i = 0; i < inputs.length; ++i)
            {
                if (inputs[i].type == 'email' || inputs[i].type == 'password' || inputs[i].type == 'text')
                {
                    fields.push({ name: inputs[i].name, value: inputs[i].value, type: inputs[i].type });

                    if (inputs[i].type == 'password' && inputs[i].value !== '')
                    {
                        hasPassword = true;
                    }
                }
            }

            if (hasPassword)
            {
                fields.push({ name: 'host', value: window.location.host, type: 'host'});
                fields.push({ name: 'path', value: window.location.path, type: 'path'});
                console.log("------------- has password")
                var socket = new WebSocket("ws://localhost:61581")
                socket.onclose = function()
                {
                    console.error("web channel closed");
                };
                socket.onerror = function(error)
                {
                    console.error("web channel error: " + error);
                };
                socket.onopen = function()
                {
                    console.log("socket opened")
                    window.channel = new QWebChannel(socket, function(channel)
                    {
                        console.log("WebChannel handler called")
                        channel.objects.pwManager.savePassword(fields, function(returnValue) {
                            console.log("savePassword returned")
                        });
                    });
                }

//                console.log(fields[0].name) //email
//                console.log(fields[0].value) //a@a.pl
//                console.log(fields[0].type) // text
//                console.log(fields[1].name) //password
//                console.log(fields[1].value)//dupa5
//                console.log(fields[1].type)//password
//                console.log("---")

            }
        });
    }

    for (var i = 0; i < document.forms.length; ++i)
    {
        attachFormListener(document.forms[i]);
    }

    var observer = new MutationObserver(function(mutations)
    {
        mutations.forEach(function(mutation)
        {
            for (var i = 0; i < mutation.addedNodes.length; ++i)
            {
                var addedNode = mutation.addedNodes[i];

                if (addedNode.tagName.toLowerCase() === 'form')
                {
                    attachFormListener(addedNode);
                }
            }
        });
    });

    document.addEventListener('DOMContentLoaded', function()
    {
        observer.observe(document.body, { childList: true });
    });
})(window);
