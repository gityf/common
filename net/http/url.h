#pragma once
#include <string>
namespace common {
    namespace net {
        class Uri {
            bool _ipv6;
            std::string _protocol;
            std::string _user;
            std::string _password;
            std::string _host;
            unsigned short int _port;
            std::string _path;
            std::string _query;
            std::string _fragment;

        public:
            Uri()  { }

            Uri(const std::string& str);
            // < to &lt;, > to &gt;...
            std::string escape(std::string const &s);

            void protocol(const std::string& protocol)
            {
                _protocol = protocol;
            }

            const std::string& protocol() const
            {
                return _protocol;
            }

            void user(const std::string& user)
            {
                _user = user;
            }

            const std::string& user() const
            {
                return _user;
            }

            void password(const std::string& password)
            {
                _password = password;
            }

            const std::string& password() const
            {
                return _password;
            }

            void host(const std::string& host)
            {
                _host = host;
            }

            const std::string& host() const
            {
                return _host;
            }

            void port(unsigned short int p)
            {
                _port = p;
            }

            unsigned short int port() const
            {
                return _port;
            }

            void path(const std::string& path)
            {
                _path = path;
            }

            const std::string& path() const
            {
                return _path;
            }

            void query(const std::string& query)
            {
                _query = query;
            }

            const std::string& query() const
            {
                return _query;
            }

            void fragment(const std::string& fragment)
            {
                _fragment = fragment;
            }

            const std::string& fragment() const
            {
                return _fragment;
            }

            std::string str() const;
        };
    }
}
// end of local file.