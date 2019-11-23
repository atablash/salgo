#pragma once


//#define FREAD  fread
//#define FWRITE fwrite

#define FREAD  fread_unlocked
#define FWRITE fwrite_unlocked






struct Buffer {
	FILE* const stream;
	const int buffer_size;

	uint8_t* const buff;
	int pos = 0;

	Buffer(FILE* a_stream, int a_buffer_size) :
			stream( a_stream ),
			buffer_size( a_buffer_size ),
			buff( new uint8_t[ buffer_size ] ) {
		setvbuf(stream, NULL, _IONBF, 0);
	}

	~Buffer() {
		delete[] buff;
	}
};










class In_Buffer : private Buffer {
public:
	In_Buffer(FILE* a_stream, int a_buffer_size) : Buffer(a_stream, a_buffer_size) {
		FREAD(buff, 1, buffer_size, stream);
	}

public:
	void prepare(int num_chars) {
		auto remaining = buffer_size - pos;
		if(remaining >= num_chars) return;

		assert(remaining >= 0); // prevent compiler warning
		memcpy(buff, buff+pos, remaining);

		int read = FREAD(buff+remaining, 1, pos, stream);
		int block_end = remaining + read;
		memset(buff + block_end, 0, buffer_size - block_end);
		pos = 0;
	}

public:
	char get_unchecked() {
		return buff[ pos++ ];
	}
};


template<class INT>
INT read_unsigned(In_Buffer& buff) {
	buff.prepare(25);

	char c = buff.get_unchecked();
	while(c < '-') c = buff.get_unchecked();

	INT r = 0;

	for(;;) {
		if(c < '0') return r;
		c -= '0';
		r = r*10 + c;
		c = buff.get_unchecked();
	}
}


template<class INT>
INT read_signed(In_Buffer& buff) {
	buff.prepare(25);

	char c = buff.get_unchecked();
	while(c < '-') c = buff.get_unchecked();

	bool minus = false;
	if(c=='-') {
		c = buff.get_unchecked();
		minus = true;
	}

	INT r = 0;

	for(;;)
	{
		if(c < '0') return minus ? -r : r;
		r = r*10 + (c-'0');
		c = buff.get_unchecked();
	}
}




In_Buffer& operator>>( In_Buffer& buff, char& c) {
	buff.prepare(1);
	c = buff.get_unchecked();
	return buff;
}



In_Buffer& operator>>( In_Buffer& buff, UI& r) { r = read_unsigned<UI>(buff); return buff; }
In_Buffer& operator>>( In_Buffer& buff, ULL& r) { r = read_unsigned<ULL>(buff); return buff; }

In_Buffer& operator>>( In_Buffer& buff, int& r) { r = read_signed<int>(buff); return buff; }
In_Buffer& operator>>( In_Buffer& buff, LL& r) { r = read_signed<LL>(buff); return buff; }

In_Buffer& operator>>( In_Buffer& buff, string& r) {
	r.clear();

	char c = buff.get_unchecked();
	while(c <= ' ') c = buff.get_unchecked();

	do {
		r.push_back( c );
		c = buff.get_unchecked();
	} while(c > ' ');

	return buff;
}



















class Out_Buffer : private Buffer {
public:
	Out_Buffer(FILE* a_stream, int a_buffer_size) : Buffer(a_stream, a_buffer_size) {}

	~Out_Buffer() {
		flush();
	}

public:
	void prepare(int num_chars) {
		if(buffer_size - pos >= num_chars) return;
		flush();
	}

	void flush() {
		FWRITE(buff, 1, pos, stream);
		pos = 0;
	}

public:
	void put_unchecked(const char& c) {
		buff[ pos++ ] = c;
	}

	void put_unchecked(const char* str, int len) {
		memcpy(buff+pos, str, len);
		pos += len;
	}
};






template<class INT>
void write_signed(Out_Buffer& buff, const INT& i) {
	buff.prepare(25);

	if(i == 0) {
		buff.put_unchecked('0');
		return;
	}

	auto ci = i;

	if(ci < 0) {
		buff.put_unchecked('-');
		ci = -ci;
	}

	const int sz = 25;
	char temp[sz];
	int pos = sz;

	while(ci) {
		temp[ --pos ] = '0' + ci%10;
		ci/=10;
	}

	buff.put_unchecked(temp+pos, sz-pos);
}



template<class INT>
void write_unsigned(Out_Buffer& buff, const INT& i) {
	buff.prepare(25);

	if(i==0) {
		buff.put_unchecked('0');
		return;
	}

	const int sz = 25;
	char temp[sz];
	int pos = sz;

	auto ci = i;

	while(ci) {
		temp[ --pos ] = '0' + ci%10;
		ci/=10;
	}

	buff.put_unchecked(temp+pos, sz-pos);
}



Out_Buffer& operator<<( Out_Buffer& buff, const char& c) {
	buff.prepare(1);
	buff.put_unchecked( c );
	return buff;
}

Out_Buffer& operator<<( Out_Buffer& buff, const UI& r) { write_unsigned<UI>(buff, r); return buff; }
Out_Buffer& operator<<( Out_Buffer& buff, const ULL& r) { write_unsigned<ULL>(buff, r); return buff; }

Out_Buffer& operator<<( Out_Buffer& buff, const int& r) { write_signed<int>(buff, r); return buff; }
Out_Buffer& operator<<( Out_Buffer& buff, const LL& r) { write_signed<LL>(buff, r); return buff; }

Out_Buffer& operator<<( Out_Buffer& buff, const char* cstr) { auto len = strlen(cstr); buff.prepare(len); buff.put_unchecked(cstr, len); return buff; }
Out_Buffer& operator<<( Out_Buffer& buff, const string& str) { buff.prepare( str.size() ); buff.put_unchecked(str.data(), str.size()); return buff; }

















#define BUFF_SIZE 32768
In_Buffer  fast_cin(  stdin,  BUFF_SIZE );
Out_Buffer fast_cout( stdout, BUFF_SIZE );
//Out_Buffer fast_cerr( stderr, BUFF_SIZE );

char fast_endl = '\n';

