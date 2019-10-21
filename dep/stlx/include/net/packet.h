#pragma once

class Packet
{
public:
	Packet() {}
	Packet(char * data) : _p(data) {}
	Packet(void * data) : _p((char*)data) {}

	char * GetData()	const { return _p; }
	bool IsEmpty() const { return _p == nullptr; }
	
#ifdef ENET_VERSION
	ENetPacket * packet;

	template<typename ...Args>
	inline void Create(Args &&...args) {
		auto sz = GetSize(std::forward<Args>(args)...);
		packet = enet_packet_create(0, sz, ENET_PACKET_FLAG_RELIABLE);
		_p = (char*) packet->data;

		_writePass(std::forward<Args>(args)...);
		ASSERT(sz == _p - (char*) packet->data);
	}
#endif

#ifdef ZMQ_VERSION
	zmq::message_t msg;

	template<typename ...Args>
	inline void Create(Args &&...args) {
		auto sz = GetSize(std::forward<Args>(args)...);
		msg.rebuild(sz);
		_p = (char*)msg.data();

		_writePass(std::forward<Args>(args)...);
		ASSERT(sz == (size_t)(_p - (char*)msg.data()));
	}
#endif

	inline void Read() {}

	template<typename ...Args>
	inline void Read(Args &&...args) {
		_read(std::forward<Args>(args)...);
	}
	
private:
	// ----------------------- SIZE ---------------------------
	// ---------------------------------------------------------
	inline static size_t GetSize() { return 0; }

	template<typename T, typename = std::enable_if<!std::is_array<T>::value>::type, typename ...Args>
	inline static size_t GetSize(const T & t, Args &&...args) {
		return _size(t) + GetSize(std::forward<Args>(args)...);
	}

	template<typename T, typename = std::enable_if<std::is_array<T>::value>::type, typename ...Args, typename = void>
	inline static size_t GetSize(const T & t, Args &&...args) {
		return _sizeArr(t) + GetSize(std::forward<Args>(args)...);
	}

	//template<typename T, size_t sz, typename ...Args>
	//inline static size_t GetSize(const T (&t)[sz], Args &&...args)
	//{ 
	//	return _size((T(&)[sz])t) + GetSize(std::forward<Args>(args)...);
	//}

	template <typename T>
	static const size_t _size(const T) { return sizeof(T); }
	template <typename T, size_t sz>
	static const size_t _sizeArr(const T (&v)[sz]) { return sizeof(v); }
	static const size_t _size(char * const str) { return strlen(str) + 2; }
	static const size_t _size(char const * const str) { return strlen(str) + 2; }
	static const size_t _size(const String & str) { return str.size() + 2; }
	template <typename T>
	static const size_t _size(const Vector<T> & v) {
		return 2 + v.Size() * sizeof(T);
	}
	static const size_t _size(const Vector<String> & v) {
		auto sz = 2 + 2 * v.Size();
		for(auto & str : v)
			sz += str.size();
		return sz;
	}


	// ----------------------- OUTPUT ---------------------------
	// ---------------------------------------------------------
	void _write() {}
	void _writePass() {}

	template<typename T, typename ...Args>
	inline void _writePass(const T t, Args &&...args)
	{
		_write(t, std::forward<Args>(args)...);
	}

	template<typename T, size_t sz, typename ...Args>
	inline void _writePass(const T (&t)[sz], Args &&...args)
	{
		memcpy(_p, t, sizeof(t));
		_p += sizeof(t);
		_writePass(std::forward<Args>(args)...);
	}

	template<typename ...Args>
	inline void _writePass(Args &&...args)
	{
		_write(std::forward<Args>(args)...);
	}

	template<typename T, typename = std::enable_if<std::is_scalar<T>::value>::type, typename ...Args>
	inline void _write(const T t, Args &&...args) {
		//Push(_p, t);
		*(T*) _p = t;
		_p += sizeof(t);
		_writePass(std::forward<Args>(args)...);
	}

	template<typename T, typename = std::enable_if<!std::is_scalar<T>::value>::type, typename ...Args, typename = void>
	inline void _write(const T t, Args &&...args) {
		memcpy(_p, &t, sizeof(t));
		_p += sizeof(t);
		_writePass(std::forward<Args>(args)...);
	}

	template<typename ...Args> void _write(char * const str, Args &&...args)
	{
		auto len = strlen(str);
		*(u16*)_p = (u16)len;
		_p += sizeof(u16);
		memcpy(_p, str, len);
		_p += len;
		_writePass(std::forward<Args>(args)...);
	}
	template<typename ...Args> void _write(char const * const str, Args &&...args)
	{
		auto len = strlen(str);
		*(u16*) _p = (u16)len;
		_p += sizeof(u16);
		memcpy(_p, str, len);
		_p += len;
		_writePass(std::forward<Args>(args)...);
	}
	template<typename ...Args> void _write(const String & str, Args &&...args) {
		auto len = str.size();
		*(u16*) _p = (u16)len;
		_p += sizeof(u16);
		memcpy(_p, str.c_str(), len);
		_p += len;
		_writePass(std::forward<Args>(args)...);
	}

	template<typename ...Args, typename T>
	void _write(const Vector<T> & v, Args &&...args) {
		*(u16*)_p = (u16)v.Size();;
		_p += sizeof(u16);

		auto sz = v.Size() * sizeof(T);
		if(sz > 0) {
			memcpy(_p, &v.Front(), sz);
			_p += sz;
		}
		_writePass(std::forward<Args>(args)...);
	}

	template<typename ...Args>
	void _write(const Vector<String> & v, Args &&...args) {
		*(u16*)_p = (u16)v.Size();;
		_p += sizeof(u16);

		for(auto & str : v) {
			auto len = str.size();
			*(u16*)_p = (u16)len;
			_p += sizeof(u16);
			memcpy(_p, str.c_str(), len);
			_p += len;
		}

		_writePass(std::forward<Args>(args)...);
	}


	// ----------------------- INPUT ---------------------------
	// ---------------------------------------------------------
	void _read() {};
	void _readPass() {};

	template<typename T, typename ...Args>
	inline void _readPass(T & t, Args &&...args)
	{
		_read(t, std::forward<Args>(args)...);
	}

	template<typename T, size_t sz, typename ...Args>
	inline void _readPass(T (&t)[sz], Args &&...args)
	{
		memcpy(t, _p, sizeof(t));
		_p += sizeof(t);
		_readPass(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	inline void _read(T & t, Args &&...args)
	{
		t = *(T*)_p;
		_p += sizeof(t);
		_readPass(std::forward<Args>(args)...);
	}

	template<typename ...Args>
	void _read(char * str, Args &&...args)
	{
		auto len = *(u16*)_p;
		_p += sizeof(u16);

		memcpy(str, _p, len);
		str [len] = 0;
		_p += len;

		_readPass(std::forward<Args>(args)...);
	}

	template<typename ...Args>
	void _read(String & str, Args &&...args) {
		auto len = *(u16*) _p;
		_p += sizeof(u16);

		str.assign(_p, len);
		_p += len;

		_readPass(std::forward<Args>(args)...);
	}

	template<typename ...Args, typename T>
	void _read(Vector<T> & v, Args &&...args) {
		auto n = *(u16*)_p;
		_p += sizeof(u16);

		v.Clear();
		auto end = _p + sizeof(T)* n;
		v.Reserve(n);
		v.EmplaceRange((T*)_p, (T*)end);
		_p = end;

		_readPass(std::forward<Args>(args)...);
	}

	template<typename ...Args>
	void _read(Vector<String> & v, Args &&...args) {
		auto n = *(u16*)_p;
		_p += sizeof(u16);

		v.Clear();
		v.Reserve(n);

		String str;
		for(size_t i = 0; i < n; i++) {
			auto len = *(u16*)_p;
			_p += sizeof(u16);

			str.assign(_p, len);
			_p += len;
			v.Emplace(str);
		}

		_readPass(std::forward<Args>(args)...);
	}

	char * _p;
};