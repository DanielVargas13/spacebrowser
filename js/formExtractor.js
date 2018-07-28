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
                console.log("has password")
                fields.push({ name: 'host', value: window.location.host, type: 'host'});
                fields.push({ name: 'path', value: window.location.pathname, type: 'path'});

                if (!window.pwManager)
                {
                    window.channel = new QWebChannel(
                        qt.webChannelTransport,
                        function(channel) {
                            window.pwManager = channel.objects.pwManager
                            window.pwManager.savePassword(fields, function(retVal) {});
                        });
                }
                else
                {
                    window.pwManager.savePassword(fields, function(retVal) {});
                }
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
