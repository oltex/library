#pragma once
#include "session.h"
#include "../../data-structure/serialize-buffer/serialize_buffer.h"
#include <list>
extern inline int test(int x, int y, int z) noexcept;
extern inline int test2(int x, int z) noexcept;

class remote_procedure_call final {
	struct header {
		unsigned short _size;
	};
    enum class type : unsigned short {
        test = 1, test2 = 2
    };
private:
    inline explicit remote_procedure_call(void) noexcept = delete;
    inline explicit remote_procedure_call(remote_procedure_call const& rhs) noexcept = delete;
    inline explicit remote_procedure_call(remote_procedure_call&& rhs) noexcept = delete;
    inline auto operator=(remote_procedure_call const& rhs) noexcept -> remote_procedure_call & = delete;
    inline auto operator=(remote_procedure_call&& rhs) noexcept -> remote_procedure_call & = delete;
    inline ~remote_procedure_call(void) noexcept = delete;
public:
    inline static void accept(session& session) noexcept {
    };
    inline static void receive(session& session) noexcept {
    	while (true) {
			if (sizeof(header) > session._recv_ring_buffer.size())
				break;
			header header_;
            session._recv_ring_buffer.peek((unsigned char*)&header_, sizeof(header));
			if (header_._size > session._recv_ring_buffer.size())
				break;
            session._recv_ring_buffer.pop(sizeof(header));
			data_structure::serialize_buffer serialize_buffer;
			session._recv_ring_buffer.peek((unsigned char*)serialize_buffer.data(), header_._size);
			session._recv_ring_buffer.pop(header_._size);
			serialize_buffer.move_rear(header_._size);
            stub(session, serialize_buffer);
		}
    };
    inline static void send(session& session, data_structure::serialize_buffer& serialize_buffer) noexcept {
        header header_;
        header_._size = serialize_buffer.rear();
        session._send_ring_buffer.push((unsigned char*)&header_, sizeof(header));
        session._send_ring_buffer.push(serialize_buffer.data(), serialize_buffer.rear());
    };
	inline static void close(session& session) noexcept {
    };
public:
    //stub
    inline static void stub(session& session, data_structure::serialize_buffer& serialize_buffer) noexcept {
    	type type_;
	    serialize_buffer >> (unsigned short&)type_;
        switch (type_) {
        case type::test: {
            int x; int y; int z;
            serialize_buffer >> x >> y >> z;
            ::test(x, y, z);
        }
            break;
		case type::test2: {
            int x; int z;
            serialize_buffer >> x >> z;
            ::test2(x, z);
        }
            break;
	    default:
            session._socket.close();
		    break;
	    }
    }
public:
    //proxy
    inline static void test(std::list<session*> session, int x, int y, int z) noexcept {
        data_structure::serialize_buffer serialize_buffer;
        serialize_buffer << static_cast<unsigned short>(type::test) << x << y << z;
        for (auto& iter : session)
            send(*iter, serialize_buffer);
    }
	inline static void test2(std::list<session*> session, int x, int z) noexcept {
        data_structure::serialize_buffer serialize_buffer;
        serialize_buffer << static_cast<unsigned short>(type::test2) << x << z;
        for (auto& iter : session)
            send(*iter, serialize_buffer);
    }
};