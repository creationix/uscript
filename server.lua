require('coro-net').createServer({
  host = "0.0.0.0",
  port = 1337
}, function (read, write, socket)
  p(socket:getpeername())
  for data in read do
    p(data)
  end
  write()
end)
print("uScript uController endpoint on port 1337")

require('weblit-app')

  .bind({
    host = "0.0.0.0",
    port = 8080
  })

  .use(require('weblit-logger'))
  .use(require('weblit-auto-headers'))
  .use(require('weblit-etag-cache'))

  .use(require('weblit-static')(module.dir .. "/www"))

  .start()
