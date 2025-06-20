#pragma once
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "mswsock.lib")
#include <MSWSock.h>
#include <intrin.h>
#include <optional>
#include <string>
#include "../overlapped/overlapped.h"
#include "../../data-structure/pair/pair.h"

namespace library::system {
	inline static void wsa_start_up(void) noexcept {
		WSAData wsadata;
		if (0 != WSAStartup(0x0202, &wsadata))
			__debugbreak();
	};
	inline static void wsa_clean_up(void) noexcept {
		if (SOCKET_ERROR == WSACleanup())
			__debugbreak();
	};

	class socket_address {
	public:
		inline explicit socket_address(void) noexcept = default;
		inline explicit socket_address(socket_address const& rhs) noexcept = default;
		inline explicit socket_address(socket_address&& rhs) noexcept = default;
		inline auto operator=(socket_address const& rhs) noexcept -> socket_address & = default;
		inline auto operator=(socket_address&& rhs) noexcept -> socket_address & = default;
		inline ~socket_address(void) noexcept = default;

		inline virtual ADDRESS_FAMILY get_family(void) const noexcept = 0;
		inline virtual int get_length(void) const noexcept = 0;
		inline virtual sockaddr& data(void) noexcept = 0;
	};
	class socket_address_ipv4 final : public socket_address {
	public:
		inline explicit socket_address_ipv4(void) noexcept
			: _sockaddr{} {
			_sockaddr.sin_family = AF_INET;
		}
		inline socket_address_ipv4(socket_address_ipv4 const& rhs) noexcept
			: _sockaddr(rhs._sockaddr) {
		};
		inline explicit socket_address_ipv4(socket_address_ipv4&& rhs) noexcept
			: _sockaddr(rhs._sockaddr) {
		};
		inline auto operator=(socket_address_ipv4 const& rhs) noexcept -> socket_address_ipv4& {
			_sockaddr = rhs._sockaddr;
			return *this;
		};
		inline auto operator=(socket_address_ipv4&& rhs) noexcept -> socket_address_ipv4& {
			_sockaddr = rhs._sockaddr;
			return *this;
		};
		inline ~socket_address_ipv4(void) noexcept = default;

		inline void set_address(unsigned long address) noexcept {
			_sockaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		}
		inline void set_address(char const* const address) noexcept {
			if (1 != inet_pton(AF_INET, address, &_sockaddr.sin_addr))
				__debugbreak();
		}
		inline auto get_address(void) const noexcept -> std::wstring {
			wchar_t string[INET_ADDRSTRLEN];
			if (0 == InetNtopW(AF_INET, &_sockaddr.sin_addr, string, INET_ADDRSTRLEN))
				__debugbreak();
			return std::wstring(string);
		}
		inline void set_port(unsigned short port) noexcept {
			_sockaddr.sin_port = htons(port);
		}
		inline auto get_port(void) const noexcept -> unsigned short {
			return ntohs(_sockaddr.sin_port);
		}

		inline virtual ADDRESS_FAMILY get_family(void) const noexcept override {
			return AF_INET;
		}
		inline virtual int get_length(void) const noexcept override {
			return sizeof(sockaddr_in);
		}
		inline virtual sockaddr& data(void) noexcept override {
			return *reinterpret_cast<sockaddr*>(&_sockaddr);
		}
	private:
		sockaddr_in _sockaddr;
	};
	class socket_address_storage final : public socket_address {
	public:
		inline explicit socket_address_storage(void) noexcept
			:_sockaddr{} {
		};
		inline explicit socket_address_storage(socket_address_storage const& rhs) noexcept
			: _sockaddr(rhs._sockaddr) {
		};
		inline explicit socket_address_storage(socket_address_storage&& rhs) noexcept
			: _sockaddr(rhs._sockaddr) {
		};
		inline auto operator=(socket_address_storage const& rhs) noexcept -> socket_address_storage& {
			_sockaddr = rhs._sockaddr;
			return *this;
		};
		inline auto operator=(socket_address_storage&& rhs) noexcept -> socket_address_storage& {
			_sockaddr = rhs._sockaddr;
			return *this;
		};
		inline ~socket_address_storage(void) noexcept = default;

		inline virtual ADDRESS_FAMILY get_family(void) const noexcept {
			return _sockaddr.ss_family;
		}
		inline virtual int get_length(void) const noexcept override {
			switch (get_family()) {
			case AF_INET:
				return sizeof(sockaddr_in);
			}
			return sizeof(sockaddr_storage);
		}
		inline virtual sockaddr& data(void) noexcept override {
			return *reinterpret_cast<sockaddr*>(&_sockaddr);
		}
	protected:
		sockaddr_storage _sockaddr;
	};

	class socket final {
	public:
		inline explicit socket(void) noexcept
			: _socket(INVALID_SOCKET) {
		}
		inline explicit socket(ADDRESS_FAMILY const address_family, int const type, int const protocol) noexcept
			: _socket(::socket(address_family, type, protocol)) {
			if (INVALID_SOCKET == _socket) {
				switch (GetLastError()) {
				case WSANOTINITIALISED:
				default:
					__debugbreak();
				}
			}
		}
		inline explicit socket(SOCKET const sock) noexcept
			: _socket(sock) {
		}
		inline explicit socket(socket const&) noexcept = delete;
		inline explicit socket(socket&& rhs) noexcept
			: _socket(rhs._socket) {
			rhs._socket = INVALID_SOCKET;
		}
		inline auto operator=(socket const&) noexcept -> socket & = delete;
		inline auto operator=(socket&& rhs) noexcept -> socket& {
			closesocket(_socket);
			_socket = rhs._socket;
			rhs._socket = INVALID_SOCKET;
			return *this;
		};
		inline ~socket(void) noexcept {
			closesocket(_socket);
		}

		inline void create(ADDRESS_FAMILY const address_family, int const type, int const protocol) noexcept {
			_socket = ::socket(address_family, type, protocol);
			if (INVALID_SOCKET == _socket) {
				switch (GetLastError()) {
				case WSANOTINITIALISED:
				default:
					__debugbreak();
				}
			}
		}
		inline auto bind(socket_address& socket_address) const noexcept -> int {
			int result = ::bind(_socket, &socket_address.data(), socket_address.get_length());
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSAEADDRINUSE:
				default:
					__debugbreak();
				}
			}
			return result;
		}
		inline auto listen(int backlog) const noexcept -> int {
			if (SOMAXCONN != backlog)
				backlog = SOMAXCONN_HINT(backlog);
			int result = ::listen(_socket, backlog);
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSAEINVAL:
				default:
					__debugbreak();
				}
			}
			return result;
		}
		inline auto accept(void) const noexcept -> data_structure::pair<socket, socket_address_ipv4> {
			socket_address_ipv4 socket_address;
			int length = sizeof(sockaddr_in);
			SOCKET sock = ::accept(_socket, &socket_address.data(), &length);
			if (INVALID_SOCKET == sock) {
				switch (GetLastError()) {
				case WSAEWOULDBLOCK:
				case WSAEINVAL:
				case WSAENOTSOCK:
				case WSAEINTR:
					break;
				default:
					__debugbreak();
				}
			}
			return data_structure::pair<socket, socket_address_ipv4>(socket(sock), socket_address);
		}
		inline auto accept_ex(socket& socket_, void* output_buffer, unsigned long address_length, unsigned long remote_address_length, overlapped& overlapeed_) noexcept {
			if (FALSE == _accept_ex(_socket, socket_.data(), output_buffer, 0, address_length, remote_address_length, nullptr, &overlapeed_.data())) {
				switch (WSAGetLastError()) {
				case ERROR_IO_PENDING:
					break;
				default:
					__debugbreak();
				}
			}
		}
		inline auto connect(socket_address& socket_address) noexcept -> int {
			int result = ::connect(_socket, &socket_address.data(), socket_address.get_length());
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSAEWOULDBLOCK:
					break;
				case WSAETIMEDOUT:
				case WSAECONNREFUSED:
					close();
					break;
				default:
					__debugbreak();
				}
			}
			return result;
		}
		inline void shutdown(int const how) const noexcept {
			if (SOCKET_ERROR == ::shutdown(_socket, how))
				__debugbreak();
		}
		inline void close(void) noexcept {
			closesocket(_socket);
			_socket = INVALID_SOCKET;
		}
		inline auto send(char const* const buffer, int const length, int const flag) noexcept -> int {
			int result = ::send(_socket, buffer, length, flag);
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSAEWOULDBLOCK:
					break;
				case WSAECONNRESET:
				case WSAECONNABORTED:
					close();
					break;
				case WSAENOTCONN:
				default:
					__debugbreak();
				}
			}
			return result;
		}
		inline auto send_to(char const* const buffer, int const length, int const flag, socket_address& socket_address) const noexcept -> int {
			return ::sendto(_socket, buffer, length, flag, &socket_address.data(), socket_address.get_length());
		}
		inline auto wsa_send(WSABUF* buffer, unsigned long count, unsigned long* byte, unsigned long flag) noexcept -> int {
			int result = WSASend(_socket, buffer, count, byte, flag, nullptr, nullptr);
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSAECONNRESET:
				case WSAECONNABORTED:
					close();
					break;
				case WSAENOTSOCK:
				default:
					__debugbreak();
				}
			}
			return result;
		}
		inline auto wsa_send(WSABUF* buffer, unsigned long count, unsigned long flag, overlapped& overlapped) noexcept -> int {
			int result = WSASend(_socket, buffer, count, nullptr, flag, &overlapped.data(), nullptr);
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSA_IO_PENDING:
				case WSAECONNRESET:
				case WSAECONNABORTED:
				case WSAEINTR:
				case WSAEINVAL:
					break;
				case WSAENOTSOCK:
				default:
					__debugbreak();
				}
			}
			return result;
		}
		inline auto receive(char* const buffer, int const length, int const flag) noexcept -> int {
			int result = ::recv(_socket, buffer, length, flag);
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSAEWOULDBLOCK:
					break;
				case WSAECONNRESET:
				case WSAECONNABORTED:
					close();
					break;
				case WSAENOTCONN:
				default:
					__debugbreak();
				}
			}
			else if (0 == result)
				close();
			return result;
		}
		inline auto receive_from(char* const buffer, int const length, int const flag, socket_address& socket_address, int& from_length) noexcept -> int {
			return ::recvfrom(_socket, buffer, length, flag, &socket_address.data(), &from_length);
		}
		inline auto wsa_receive(WSABUF* buffer, unsigned long count, unsigned long* byte, unsigned long* flag) noexcept -> int {
			int result = WSARecv(_socket, buffer, count, byte, flag, nullptr, nullptr);
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSAECONNRESET:
				case WSAECONNABORTED:
					close();
					break;
				case WSAENOTSOCK:
				default:
					__debugbreak();
				}
			}
			return result;
		}
		inline auto wsa_receive(WSABUF* buffer, unsigned long count, unsigned long* flag, overlapped& overlapped) noexcept -> int {
			int result = WSARecv(_socket, buffer, count, nullptr, flag, &overlapped.data(), nullptr);
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSA_IO_PENDING:
				case WSAECONNRESET:
				case WSAECONNABORTED:
					break;
				case WSAENOTSOCK:
				default:
					__debugbreak();
				}
			}
			return result;
		}
		inline void cancel_io(void) const noexcept {
			CancelIo(reinterpret_cast<HANDLE>(_socket));
		}
		inline void cancel_io_ex(void) const noexcept {
			CancelIoEx(reinterpret_cast<HANDLE>(_socket), nullptr);
		}
		inline void cancel_io_ex(overlapped overlapped) const noexcept {
			CancelIoEx(reinterpret_cast<HANDLE>(_socket), &overlapped.data());
		}
		inline bool wsa_get_overlapped_result(overlapped& overlapped, unsigned long* transfer, bool const wait, unsigned long* flag) noexcept {
			return WSAGetOverlappedResult(_socket, &overlapped.data(), transfer, wait, flag);
		}
		inline void set_option_tcp_nodelay(int const enable) const noexcept {
			set_option(IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char const*>(&enable), sizeof(int));
		}
		inline void set_option_linger(unsigned short const onoff, unsigned short const time) const noexcept {
			LINGER linger{ onoff , time };
			set_option(SOL_SOCKET, SO_LINGER, reinterpret_cast<char const*>(&linger), sizeof(LINGER));
		}
		inline void set_option_broadcast(unsigned long const enable) const noexcept {
			set_option(SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char const*>(&enable), sizeof(unsigned long));
		}
		inline void set_option_send_buffer(int const size) const noexcept {
			set_option(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char const*>(&size), sizeof(size));
		}
		inline void set_option_receive_buffer(int const size) const noexcept {
			set_option(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char const*>(&size), sizeof(size));
		}
		inline void set_option_update_accept_context(socket& socket_) const noexcept {
			set_option(SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&socket_.data()), sizeof(SOCKET));
		}
		inline void set_option(int const level, int const name, char const* value, int const length) const noexcept {
			if (SOCKET_ERROR == setsockopt(_socket, level, name, value, length))
				__debugbreak();
		}
		inline void io_control_nonblocking(unsigned long const enable) const noexcept {
			io_control(FIONBIO, enable);
		}
		inline void io_control(long const cmd, unsigned long arg) const noexcept {
			if (SOCKET_ERROR == ioctlsocket(_socket, cmd, &arg))
				__debugbreak();
		}
		inline void wsa_io_control_acccept_ex(void) noexcept {
			GUID guid = WSAID_ACCEPTEX;
			unsigned long byte_returned;
			wsa_io_control(SIO_GET_EXTENSION_FUNCTION_POINTER, reinterpret_cast<void*>(&guid), sizeof(GUID), reinterpret_cast<void*>(&_accept_ex), sizeof(LPFN_ACCEPTEX), byte_returned);
		}
		inline void wsa_io_control_disconnect_ex(void) noexcept {
			GUID guid = WSAID_DISCONNECTEX;
			unsigned long byte_returned;
			wsa_io_control(SIO_GET_EXTENSION_FUNCTION_POINTER, reinterpret_cast<void*>(&guid), sizeof(GUID), reinterpret_cast<void*>(&_disconnect_ex), sizeof(LPFN_DISCONNECTEX), byte_returned);
		}
		inline void wsa_io_control(unsigned long control_code, void* in_buffer, unsigned long in_buffer_size, void* out_buffer, unsigned long out_buffer_size, unsigned long& byte_returned) noexcept {
			if (SOCKET_ERROR == ::WSAIoctl(_socket, control_code, in_buffer, in_buffer_size, out_buffer, out_buffer_size, &byte_returned, nullptr, nullptr))
				__debugbreak();
		}
		inline auto get_local_socket_address(void) const noexcept -> std::optional<socket_address_ipv4> {
			socket_address_ipv4 socket_address;
			int length = socket_address.get_length();
			if (SOCKET_ERROR == getsockname(_socket, &socket_address.data(), &length)) {
				switch (GetLastError()) {
				default:
					break;
#pragma warning(suppress: 4065)
				}
				return std::nullopt;
			}
			return socket_address;
		}
		inline auto get_remote_socket_address(void) const noexcept -> std::optional<socket_address_ipv4> {
			socket_address_ipv4 socket_address;
			int length = socket_address.get_length();
			if (SOCKET_ERROR == getpeername(_socket, &socket_address.data(), &length)) {
				switch (GetLastError()) {
				default:
					break;
#pragma warning(suppress: 4065)
				}
				return std::nullopt;
			}
			return socket_address;
		}
		inline auto data(void) noexcept -> SOCKET& {
			return _socket;
		}
	private:
		SOCKET _socket;
		inline static LPFN_ACCEPTEX _accept_ex;
		inline static LPFN_DISCONNECTEX _disconnect_ex;
	};
}


//class SocketUtils
//{
//public:
//	//네트워크 Init 함수
//	static void WSAInit();
//	static bool WSAClear();
//	static bool SocketWIndowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
//
//
//	static SOCKET CreateSocket();
//	static bool Bind(SOCKET socket, SOCKADDR_IN& server_addr);
//	static bool BindAnyAddr(SOCKET socket, uint16 port);
//	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
//	static bool Connect(SOCKET socket, SOCKADDR_IN& server_addr);
//
//	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 timeout);
//	static bool SetKeepAlive(SOCKET socket, bool onoff);
//	static bool SetSendBufferSize(SOCKET socket, int buf_size);
//	static bool SetRecvBufferSize(SOCKET socket, int buf_size);
//	static bool SetNagle(SOCKET socket, bool onoff);
//
//	__forceinline static bool CopySocketAttribute(SOCKET client_socket, SOCKET listen_socket)
//	{
//		return ::setsockopt(client_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&listen_socket), sizeof(SOCKET)) != SOCKET_ERROR;
//	}
//
//public:
//	static LPFN_DISCONNECTEX   DisconnectEx;
//	static LPFN_ACCEPTEX      AcceptEx;
//
//};
//
//LPFN_DISCONNECTEX   SocketUtils::DisconnectEx = nullptr;
//LPFN_ACCEPTEX      SocketUtils::AcceptEx = nullptr;
//
//void SocketUtils::WSAInit()
//{
//	int32 err_code;
//	WSADATA wsa_data;
//
//	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
//	{
//		WCHAR log_buff[df_LOG_BUFF_SIZE];
//		err_code = WSAGetLastError();
//		wsprintf(log_buff, L"WSAStartUp Fail [ErrCode:%d]", err_code);
//		g_logutils->Log(NET_LOG_DIR, NET_FILE_NAME, L"WSAStartUp Success");
//		__debugbreak();
//	}
//
//	g_logutils->Log(NET_LOG_DIR, NET_FILE_NAME, L"WSAStartUp Success");
//
//	SOCKET socket = CreateSocket();
//
//	if (SocketWIndowsFunction(socket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)) == false)
//	{
//		WCHAR log_buff[df_LOG_BUFF_SIZE];
//		err_code = WSAGetLastError();
//		wsprintf(log_buff, L"AcceptEx Function Setting Fail [ErrCode:%d]", err_code);
//		wcout << log_buff << '\n';
//		__debugbreak();
//	}
//
//	if (SocketWIndowsFunction(socket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)) == false)
//	{
//		WCHAR log_buff[df_LOG_BUFF_SIZE];
//		err_code = WSAGetLastError();
//		wsprintf(log_buff, L"DisconnectEx Function Setting Fail [ErrCode:%d]", err_code);
//		wcout << log_buff << '\n';
//		__debugbreak();
//	}
//
//	closesocket(socket);
//
//	return;
//}
//
//bool SocketUtils::WSAClear()
//{
//	return ::WSACleanup();
//}
//
//bool SocketUtils::SocketWIndowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
//{
//	DWORD bytes = 0;
//	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), &bytes, NULL, NULL);
//}
//
//SOCKET SocketUtils::CreateSocket()
//{
//	return ::socket(AF_INET, SOCK_STREAM, 0);
//}
//
//bool SocketUtils::Bind(SOCKET socket, SOCKADDR_IN& server_addr)
//{
//	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(SOCKADDR_IN));
//}
//
//bool SocketUtils::BindAnyAddr(SOCKET socket, uint16 port)
//{
//	SOCKADDR_IN sock_addr;
//	sock_addr.sin_port = ::htons(port);
//	sock_addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
//	sock_addr.sin_family = AF_INET;
//
//	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<SOCKADDR*>(&sock_addr), sizeof(SOCKADDR_IN));
//}
//
//bool SocketUtils::Listen(SOCKET socket, int32 backlog)
//{
//	return SOCKET_ERROR != ::listen(socket, backlog);
//}
//
//bool SocketUtils::Connect(SOCKET socket, SOCKADDR_IN& server_addr)
//{
//	return SOCKET_ERROR != connect(socket, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(SOCKADDR_IN));
//}
//
//bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 timeout)
//{
//	LINGER linger;
//	linger.l_onoff = onoff;
//	linger.l_linger = timeout;
//	return SOCKET_ERROR != ::setsockopt(socket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&linger), sizeof(LINGER));
//}
//
//bool SocketUtils::SetKeepAlive(SOCKET socket, bool onoff)
//{
//	return SOCKET_ERROR != ::setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&onoff), sizeof(onoff));
//}
//
//bool SocketUtils::SetSendBufferSize(SOCKET socket, int buf_size)
//{
//	return SOCKET_ERROR != ::setsockopt(socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&buf_size), sizeof(buf_size));
//}
//
//bool SocketUtils::SetRecvBufferSize(SOCKET socket, int buf_size)
//{
//	return SOCKET_ERROR != ::setsockopt(socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&buf_size), sizeof(buf_size));
//}
//
//bool SocketUtils::SetNagle(SOCKET socket, bool onoff)
//{
//	return SOCKET_ERROR != ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&onoff), sizeof(onoff));
//}
//
//
//
//
//
////Session MAX를 stack size로 처리하고 있다.
//if (_session_idx_stack.pop(session_idx) == false)
//{
//	session = _session_pool->Alloc();
//	//소켓 생성 및 RecvBuffer 할당만 이루어진다.
//	session->SessionNetTempInit();
//
//	if (_iocp_core.Register(session) == false)
//	{
//		API_BREAKLOG(dfLOG_LEVEL_SYSTEM, session->_session_id, err_code, L"IOCP Core Socket Register Fail");
//	}
//}
//else
//{
//	session = &_sessions[session_idx];
//
//	//Data초기화 및 AcceptOverlapped 설정, socket 생성
//	session->SessionNetPreInit((session_idx) | ((AtomicIncrement64(&_session_cnt) - 1) << 16));
//	if (_iocp_core.Register(session) == false)
//	{
//		API_BREAKLOG(dfLOG_LEVEL_SYSTEM, session->_session_id, err_code, L"IOCP Core Socket Register Fail");
//	}
//}
//
//
//if (SocketUtils::AcceptEx(_listen_socket, session->_socket, session->_recv_buf->GetWritePos(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &transferred, &session->_recv_overlapped) == false)
//{
//	err_code = ::WSAGetLastError();
//
//	if (err_code != WSA_IO_PENDING)
//	{
//		if (InterlockedAnd8(&_exit_flag, 1) == 1)
//		{
//			AtomicDecrement64(&_alloc_session_num);
//			_session_idx_stack.push(session_idx);
//			return;
//		}
//		LIB_BREAKLOG(dfLOG_LEVEL_SYSTEM, session->_session_id, err_code, L"AcceptEx Fail");
//	}
//
//}
//
////listen_socket으로 부터 특성 상속받기
//if (SocketUtils::CopySocketAttribute(session->_socket, _listen_socket) == false)
//{
//	API_LOG(dfLOG_LEVEL_SYSTEM, session->_session_id, err_code, L"ClientSocket Copy Attributes Fail");
//	return false;
//}
//
//if (::getpeername(session->_socket, reinterpret_cast<SOCKADDR*>(&client_addr), &sock_addr_len) == SOCKET_ERROR)
//{
//	API_LOG(dfLOG_LEVEL_SYSTEM, session->_session_id, err_code, L"ClientSocket Peer Name Fail");
//	return false;
//}