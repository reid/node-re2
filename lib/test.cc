#include "./wrapped_re2.h"

#include <vector>

#include <node_buffer.h>


using std::vector;

using v8::Local;
using v8::String;


NAN_METHOD(WrappedRE2::Test) {
	NanScope();

	// unpack arguments

	WrappedRE2* re2 = ObjectWrap::Unwrap<WrappedRE2>(args.This());
	if (!re2) {
		NanReturnValue(NanNew(false));
	}

	vector<char> buffer;

	char*  data;
	size_t size, lastIndex = 0;
	bool   isBuffer = false;

	if (node::Buffer::HasInstance(args[0])) {
		isBuffer = true;
		size = node::Buffer::Length(args[0]);
		if (re2->global) {
			if (re2->lastIndex > size) {
				re2->lastIndex = 0;
				NanReturnValue(NanNew(false));
			}
			lastIndex = re2->lastIndex;
		}
		data = node::Buffer::Data(args[0]);
	} else {
		if (re2->global && re2->lastIndex) {
			String::Value s(args[0]->ToString());
			if (re2->lastIndex > s.length()) {
				re2->lastIndex = 0;
				NanReturnValue(NanNew(false));
			}
			Local<String> t(NanNew<String>(*s + re2->lastIndex));
			buffer.resize(t->Utf8Length() + 1);
			t->WriteUtf8(&buffer[0]);
		} else {
			Local<String> t(args[0]->ToString());
			buffer.resize(t->Utf8Length() + 1);
			t->WriteUtf8(&buffer[0]);
		}
		size = buffer.size() - 1;
		data = &buffer[0];
	}

	// actual work

	if (re2->global) {
		StringPiece match;
		if (re2->regexp.Match(StringPiece(data, size), lastIndex, size, RE2::UNANCHORED, &match, 1)) {
			re2->lastIndex += isBuffer ? match.data() - data + match.size() - lastIndex :
				getUtf16Length(data, match.data() + match.size());
			NanReturnValue(NanNew(true));
		}
		re2->lastIndex = 0;
		NanReturnValue(NanNew(false));
	}

	NanReturnValue(NanNew(re2->regexp.Match(StringPiece(data, size), 0, size, RE2::UNANCHORED, NULL, 0)));
}
