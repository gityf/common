
#include <stdexcept>
#include <sstream>
#include <cctype>
#include "url.h"
namespace common {
    namespace net {
        namespace
        {
            void throwInvalid(const std::string& uri)
            {
                throw std::runtime_error("invalid uri <" + uri + '>');
            }
        }

        Uri::Uri(const std::string& uri)
            : _ipv6(false),
            _port(0)
        {
            enum {
                state_0,
                state_protocol,
                state_postprotocol,
                state_postprotocol2,
                state_postprotocol3,
                state_user_or_host,
                state_password_or_port,
                state_password,
                state_host,
                state_ipv6,
                state_ipv6ok,
                state_ipv6end,
                state_port,
                state_path,
                state_query,
                state_fragment
            } state = state_0;

            std::string token;
            bool hasPort = false;

            for (std::string::const_iterator it = uri.begin(); it != uri.end(); ++it)
            {
                char ch = *it;
                switch (state)
                {
                case state_0:
                    if (std::isalpha(ch)) {
                        _protocol = ch;
                        state = state_protocol;
                    }
                    else if (!std::isspace(ch))
                        throwInvalid(uri);
                    break;

                case state_protocol:
                    if (std::isalpha(ch))
                        _protocol += ch;
                    else if (ch == ':')
                        state = state_postprotocol;
                    else
                        throwInvalid(uri);
                    break;

                case state_postprotocol:
                    if (ch == '/')
                        state = state_postprotocol2;
                    else
                        throwInvalid(uri);
                    break;

                case state_postprotocol2:
                    if (ch == '/')
                        state = state_postprotocol3;
                    else
                        throwInvalid(uri);
                    break;

                case state_postprotocol3:
                    if (ch == '[') {
                        _ipv6 = true;
                        state = state_ipv6;
                    }
                    else {
                        _user = ch;
                        state = state_user_or_host;
                    }
                    break;

                case state_user_or_host:
                    if (ch == ':')
                        state = state_password_or_port;
                    else if (ch == '/') {
                        _host = _user;
                        _user.clear();
                        _path = ch;
                        state = state_path;
                    }
                    else if (ch == '@')
                        state = state_host;
                    else
                        _user += ch;
                    break;

                case state_password_or_port:
                    if (ch == '@') {
                        _port = 0;
                        state = state_host;
                    }
                    else if (ch == '/') {
                        _host = _user;
                        _user.clear();
                        _password.clear();
                        _path = ch;
                        state = state_path;
                    }
                    else if (std::isdigit(ch)) {
                        hasPort = true;
                        _password += ch;
                        _port = _port * 10 + ch - '0';
                    }
                    else {
                        _port = 0;
                        hasPort = false;
                        _password += ch;
                        state = state_password;
                    }
                    break;

                case state_password:
                    if (ch == '@')
                        state = state_host;
                    else
                        _password += ch;
                    break;

                case state_host:
                    if (ch == '/') {
                        _path = ch;
                        state = state_path;
                    }
                    else if (ch == ':')
                        state = state_port;
                    else if (_host.empty() && ch == '[') {
                        _ipv6 = true;
                        state = state_ipv6;
                    }
                    else
                        _host += ch;
                    break;

                case state_ipv6:
                    if (ch == ':') {
                        _host += ch;
                        state = state_ipv6ok;
                    }
                    else if (std::isdigit(ch)
                        || (ch >= 'a' && ch <= 'f')
                        || (ch >= 'F' && ch <= 'F'))
                        _host += ch;
                    else
                        throwInvalid(uri);
                    break;

                case state_ipv6ok:
                    if (ch == ']')
                        state = state_ipv6end;
                    else if (std::isdigit(ch)
                        || (ch >= 'a' && ch <= 'f')
                        || (ch >= 'F' && ch <= 'F')
                        || ch == ':')
                        _host += ch;
                    else
                        throwInvalid(uri);
                    break;

                case state_ipv6end:
                    if (ch == ':') {
                        hasPort = true;
                        state = state_port;
                    }
                    else if (ch == '/') {
                        _path = ch;
                        state = state_path;
                    }
                    else
                        throwInvalid(uri);
                    break;

                case state_port:
                    if (ch == '/') {
                        _path = ch;
                        state = state_path;
                    }
                    else if (std::isdigit(ch)) {
                        hasPort = true;
                        _port = _port * 10 + ch - '0';
                    }
                    else
                        throwInvalid(uri);
                    break;

                case state_path:
                    if (ch == '?')
                        state = state_query;
                    else if (ch == '#')
                        state = state_fragment;
                    else
                        _path += ch;
                    break;

                case state_query:
                    if (ch == '#')
                        state = state_fragment;
                    else
                        _query += ch;
                    break;

                case state_fragment:
                    _fragment += ch;
                    break;
                }
            }

            switch (state) {
            case state_port:
            case state_host:
            case state_path:
            case state_query:
            case state_fragment:
                break;

            case state_user_or_host:
                _host = _user;
                _user.clear();
                break;

            default:
                throwInvalid(uri);
            }

            if (!hasPort) {
                if (_protocol == "http")
                    _port = 80;
                else if (_protocol == "https")
                    _port = 443;
                else if (_protocol == "ftp")
                    _port = 21;
            }
        }

        std::string Uri::str() const
        {
            std::ostringstream s;
            s << _protocol << "://";
            if (!_user.empty() || !_password.empty()) {
                s << _user;
                if (!_password.empty())
                    s << ':' << _password;
                s << '@';
            }

            if (_ipv6)
                s << '[' << _host << ']';
            else
                s << _host;

            if (!(_port == 0
                || (_protocol == "http"  && _port == 80)
                || (_protocol == "https" && _port == 443)
                || (_protocol == "ftp"   && _port == 21)))
                s << ':' << _port;

            s << _path;
            if (!_query.empty())
                s << '?' << _query;
            if (!_fragment.empty())
                s << '#' << _fragment;

            return s.str();
        }
        std::string Uri::escape(std::string const &s) {
            std::string content;
            unsigned i, len = s.size();
            content.reserve(len * 3 / 2);
            for (i = 0; i < len; i++) {
                char c = s[i];
                switch (c){
                case '<': content += "&lt;"; break;
                case '>': content += "&gt;"; break;
                case '&': content += "&amp;"; break;
                case '\"': content += "&quot;"; break;
                default: content += c;
                }
            }
            return content;
        }
    }
}