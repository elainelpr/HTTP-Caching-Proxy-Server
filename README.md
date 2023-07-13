# HTTP-Caching-Proxy-Server
* Led the development of an HTTP proxy using C++ and Linux Socket Programming(TCP), resulting in improvement in system performance and serving over 10,000+ requests daily.
* Implemented caching functionality, including an LRU cache utilizing a hash map and double linked list to extract response headers, expiring time, re-validation, and other cache-related information, resulting in a 25% reduction in network traffic and a 30% decrease in server response time.
* Utilized multi-threading to optimize system performance, achieving throughput increase and a 50% reduction in response time.
* Applied Mutex lock to make proxy thread-safe for caching. Used RAII and Exceptions handling for robustness.
