# Test Cases

### GET request

[http://info.cern.ch/hypertext/WWW/TheProject.html](http://info.cern.ch/hypertext/WWW/TheProject.html)

- my proxy can send back 200 OK response
- this response can be stored in my cache , since it doesn’t have cache-control header field
- it must require revalidation for each request

[http://go.com/](http://go.com/)

- my proxy can send back 200 OK response
- this response can be stored in my cache, since it doesn’t have cache-control header field
- it doesn’t require revalidation within 1812 seconds(max-age)

[http://feedburner.google.com/fb/i/icn/feed-icon-10x10.gif](http://feedburner.google.com/fb/i/icn/feed-icon-10x10.gif)

- my proxy can send back 200 OK response
- this response can be stored in my cache, since it doesn’t have cache-control header field
- it must require revalidation since it max-age field is 0

### POST request

[http://httpbin.org/post](http://httpbin.org/post)

- test it in postman and set key and value
- my proxy can send back 200 OK response

### CONNECT request

[https://www.google.com/](https://www.google.com/)

[https://www.cplusplus.com/](https://www.cplusplus.com/)

- my proxy can send back 200 OK response