#include "MNBasicLib.h"
#include "MNFiber.h"
#include "MNGlobal.h"
#include "MNString.h"
#include "MNClosure.h"
#include "MNTable.h"
#include "MNArray.h"
#include "MNOpenGL.h"
#include "MNUserData.h"

#if defined(PLATFORM_WIN32)
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include <stdlib.h>
#include <math.h>

struct CommonLib
{
    static void push_closure(MNFiber* fiber, NativeFunc func)
    {
        MNClosure* closure = new MNClosure(MNObject::CFunction(func));
        MNObject obj(TObjectType::TClosure, closure->link(fiber->global())->getReferrer());
        fiber->push(obj);
    }
    
    static void make_metaTable(MNFiber* fiber, const char* name)
    {
        MNTable* table = new MNTable(1);
        MNObject obj(TObjectType::TTable, table->link(fiber->global())->getReferrer());
        fiber->push_string(name);
        fiber->push(obj);
        fiber->store_global();
    }
    
	static bool print(MNFiber* fiber)
	{
		tsize sz = fiber->localSize();
		for (tsize i = 1; i < sz; ++i)
		{
			fiber->load_stack(i);
			fiber->tostring();
			MNString* str = fiber->get(-1).toString();
			printf(str->ss().c_str());
			fiber->pop(1);
		}
		printf("\n");
		return false;
	}

	static bool bind(MNFiber* fiber)
	{
		MNObject cls = fiber->get(1);
		MNObject obj = fiber->get(2);
		MNClosure* closure = cls.toClosure();
		if (closure) closure->bindThis(obj);
		return false;
	}

	static bool castFloat(MNFiber* fiber)
	{
		fiber->push_real(fiber->get(1).toReal());
		return true;
	}

	static bool castInt(MNFiber* fiber)
	{
		fiber->push_integer(fiber->get(1).toInt());
		return true;
	}

	static bool allocate(MNFiber* fiber)
	{
		const MNObject& size = fiber->get(1);
		if (!size.isInt()) return false;
		fiber->push_userdata((tsize)size.toInt(), NULL);
		return true;
	}

	static bool _typeof(MNFiber* fiber)
	{
		const MNObject& val = fiber->get(1);
		switch (val.getType())
		{
		case TObjectType::TInt      : fiber->push_string("int"); break;
		case TObjectType::TNull     : fiber->push_string("null"); break;
		case TObjectType::TReal     : fiber->push_string("float"); break;
		case TObjectType::TString   : fiber->push_string("string"); break;
		case TObjectType::TPointer  : fiber->push_string("pointer"); break;
		case TObjectType::TFunction : fiber->push_string("function"); break;
		case TObjectType::TCFunction: fiber->push_string("cfunction"); break;
		default:
			{
				fiber->load_stack(1);
				fiber->push_string("type");
				fiber->load_field();
			}
			break;
		}
		return true;
	}

	static bool dofile(MNFiber* fiber)
	{
		MNObject arg1 = fiber->get(1);
		if (!arg1.isString()) return false;

		MNString* path = arg1.toString();
		fiber->push_bool(fiber->dofile(path->ss().str()));

		return true;
	}
    
    static bool dotext(MNFiber* fiber)
    {
        MNObject arg1 = fiber->get(1);
        if (!arg1.isString()) return false;
        
        MNString* text = arg1.toString();
        fiber->push_bool(fiber->dotext(text->ss().str()));
        
        return true;
    }

	static bool garbageCollect(MNFiber* fiber)
	{
		fiber->push_integer(fiber->global()->GC());
		return true;
	}

	static bool setMeta(MNFiber* fiber)
	{
		MNCollectable* obj = fiber->get(1).toCollectable();
		if (!obj) return false;
		obj->setMeta(fiber->get(2));
		fiber->load_stack(1);
		return true;
	}

	static bool getMeta(MNFiber* fiber)
	{
		MNCollectable* obj = fiber->get(1).toCollectable();
		if (!obj) return false;
		fiber->push(obj->getMeta());
		return true;
	}

	static bool delegator(MNFiber* fiber)
	{
		MNClosure* closure = fiber->get(1).toClosure();
		MNObject delegator;
		closure->clone(delegator);
		delegator.toClosure()->bindThis(fiber->get(2));
		fiber->push(delegator);
		return true;
	}
    
    static bool getSeconds(MNFiber* fiber)
    {
        treal seconds = 0;
#if defined(PLATFORM_WIN32)
        static tuint32 startTime = GetTickCount();
        unsigned int count = GetTickCount() - startTime;
        seconds = ((float)count)/1000.0f;
#else
        struct timeval tick;
        gettimeofday(&tick, 0);
        
        static tuint32 startTime = tick.tv_sec;
        seconds = (float)(tick.tv_sec - startTime);
        seconds += ((float)tick.tv_usec)/1000000.0f;
#endif
        fiber->push_real(seconds);
        return true;
    }
    
    static bool math_sqrt(MNFiber* fiber)
    {
        float a = sqrtf((tfloat)fiber->get(1).toReal());
        fiber->push_real(a);
        return true;
    }
    
    static bool math_pow(MNFiber* fiber)
    {
        float a = powf((tfloat)fiber->get(1).toReal(), (tfloat)fiber->get(2).toReal());
        fiber->push_real(a);
        return true;
    }
    
    static bool closure_call(MNFiber* fiber)
    {
        fiber->call(fiber->localSize() - 1, true);
        return true;
    }
    
    static bool closure_compileFile(MNFiber* fiber)
    {
        fiber->load_stack(1);
        fiber->tostring();
        MNString* str = fiber->get(-1).toString();
        
        MNObject func;
        if (!fiber->compileFile(func, str->ss().str())) return false;
        
        fiber->push_closure(NULL); //! [closure]
        MNClosure* closure = fiber->get(-1).toClosure();
        closure->setFunc(func);
        return true;
    }
    
    static bool fiber_new(MNFiber* fiber)
    {
        MNFiber* newFiber = new MNFiber(fiber->global());
        newFiber->setMeta(fiber->get(0));
        newFiber->setStatus(MNFiber::Suspend);
        
        newFiber->push(fiber->get(1));
        newFiber->push(fiber->get(1));
        newFiber->load_stack(0);
		newFiber->push_null();
        newFiber->enterCall(2, true);
        fiber->push(MNObject(TObjectType::TFiber, newFiber->getReferrer()));
        return true;
    }
    
    static bool fiber_next(MNFiber* fiber)
    {
        MNFiber* newFiber = fiber->get(0).toFiber();
        if (!newFiber) return false;
        else if (newFiber->getStatus() == MNFiber::Stop)   return false;
        else if (newFiber->getStatus() == MNFiber::Resume) return false;
        
        newFiber->setStatus(MNFiber::Resume);
        
        newFiber->set(-1, fiber->get(1));
        tint status = newFiber->excuteCall();
        
        newFiber->setStatus(status);
        fiber->push_bool(status == MNFiber::Suspend);
        return true;
    }
    
    static bool fiber_reset(MNFiber* fiber)
    {
        MNFiber* newFiber = fiber->get(0).toFiber();
        if (!newFiber) return false;
        else if (newFiber->getStatus() != MNFiber::Stop)
        {
            MNFiber::CallInfo* info = NULL;
            do
            {
                info = newFiber->returnCall(false);
            } while (info->closure != NULL);
        }
        
        newFiber->setStatus(MNFiber::Suspend);
        newFiber->pop(1);
        newFiber->push(newFiber->getAt(1));
        newFiber->load_stack(0);
		newFiber->push_null();
		newFiber->enterCall(2, true);
        return false;
    }
    
    static bool fiber_value(MNFiber* fiber)
    {
        MNFiber* newFiber = fiber->get(0).toFiber();
        if (!newFiber) return false;
        fiber->push(newFiber->get(-1));
        return true;
    }
    
    static bool array_count(MNFiber* fiber)
    {
        const MNObject& obj = fiber->get(0);
        MNArray* arr = obj.toArray();
        if (!arr) return false;
        fiber->push_integer(arr->count());
        return true;
    }
    
    static bool array_add(MNFiber* fiber)
    {
        const MNObject& obj = fiber->get(0);
        MNArray* arr = obj.toArray();
        if (!arr) return false;
        arr->add(fiber->get(1));
        return false;
    }
    
    static bool array_remove(MNFiber* fiber)
    {
        const MNObject& obj = fiber->get(0);
        MNArray* arr = obj.toArray();
        if (!arr) return false;
        fiber->push_bool(arr->remove(fiber->get(1)));
        return true;
    }
    
    static bool array_clear(MNFiber* fiber)
    {
        const MNObject& obj = fiber->get(0);
        MNArray* arr = obj.toArray();
        if (!arr) return false;
        arr->clear();
        return false;
    }
    
    static bool array_iterate(MNFiber* fiber)
    {
        MNArray* arr = fiber->get(0).toArray();
        if (!arr) return false;
        
        if (!fiber->get(1).isClosure()) return false;
        
        tsize itor = 0;
        MNObject val;
        while (arr->iterate(itor, val))
        {
            fiber->load_stack(1);
            fiber->load_stack(0);
            fiber->push_integer(itor - 1);
            fiber->push(val);
            fiber->call(3, true);
            bool ret = fiber->get(-1).toBool(true);
            fiber->pop(1);
            if (!ret) break;
        }
        return false;
    }
    
    static bool table_has(MNFiber* fiber)
    {
        const MNObject& obj = fiber->get(0);
        MNTable* tbl = obj.toTable();
        if (!tbl) return false;
        fiber->push_bool(tbl->hasKey(fiber->get(1)));
        return true;
    }
    
    static bool table_count(MNFiber* fiber)
    {
        const MNObject& obj = fiber->get(0);
        MNTable* tbl = obj.toTable();
        if (!tbl) return false;
        fiber->push_integer(tbl->count());
        return true;
    }
    
    static bool table_insert(MNFiber* fiber)
    {
        fiber->store_raw_field(true);
        return false;
    }
    
    static bool table_capacity(MNFiber* fiber)
    {
        const MNObject& obj = fiber->get(0);
        MNTable* tbl = obj.toTable();
        if (!tbl) return false;
        fiber->push_integer(tbl->capacity());
        return true;
    }
    
    static bool table_iterate(MNFiber* fiber)
    {
        MNTable* tbl = fiber->get(0).toTable();
        if (!tbl) return false;
        
        if (!fiber->get(1).isClosure()) return false;
        
        tsize itor = 0;
        MNObject key, val;
        while (tbl->iterate(itor, key, val))
        {
            if (key.isNull()) continue;
            fiber->load_stack(1);
            fiber->load_stack(0);
            fiber->push(key);
            fiber->push(val);
            fiber->call(3, true);
            bool ret = fiber->get(-1).toBool(true);
            fiber->pop(1);
            if (!ret) break;
        }
        return false;
    }

	static bool opengl_clearColor(MNFiber* fiber)
	{
		float r = fiber->get(1).toReal();
		float g = fiber->get(2).toReal();
		float b = fiber->get(3).toReal();
		float a = fiber->get(4).toReal();
		glClearColor(r, g, b, a);
		return false;
	}
    
	static bool opengl_clear(MNFiber* fiber)
	{
		GLbitfield bufferBit = 0;

		if (fiber->get(1).toBool()) bufferBit |= GL_COLOR_BUFFER_BIT;
		if (fiber->get(2).toBool()) bufferBit |= GL_DEPTH_BUFFER_BIT;

		glClear(bufferBit);
		return false;
	}

    struct MNSocket
    {
        int sock;
        bool connected;
        MNSocket():sock(0), connected(false) {}
    };
    
    static void socket_finalizer(void* data, tsize size)
    {
        TSocket* socket = (TSocket*)data;
        socket->~TSocket();
    }
    
    static bool socket_connect(MNFiber* fiber)
    {
        TSocket* socket = new(fiber->push_userdata(sizeof(TSocket), &socket_finalizer)) TSocket();
        MNString* arg1 = fiber->get(1).toString();
        
        tstring  address = arg1? arg1->ss().str() : "";
        tinteger port = fiber->get(2).toInt();
        socket->connect(address, port, true);
        
        return true;
    }
    
    static bool socket_send(MNFiber* fiber)
    {
        MNUserData* userData = fiber->get(1).toUserData();
        if (userData == NULL)
        {
            fiber->push_bool(false);
            return true;
        }
        
        TSocket* socket = (TSocket*)userData->getData();
        
        fiber->load_stack(2);
        fiber->tostring();
        MNString* str = fiber->get(-1).toString();
        
        bool ret = socket->sendBuffer((tbyte*)str->ss().c_str(), str->ss().str().length());
        fiber->push_bool(ret);
        return true;
    }
    
    static bool socket_read(MNFiber* fiber)
    {
        MNUserData* userData = fiber->get(1).toUserData();
        if (userData == NULL)
        {
            fiber->push_null();
            return true;
        }
        
        TSocket* socket = (TSocket*)userData->getData();
        
        const tint32 bufSize = 1024;
        tbyte buf[bufSize] = {0};

        bool ret = socket->readBuffer(buf, bufSize);
        if (ret)
        {
            fiber->push_string((char*)&buf[0]);
        }
        else
        {
            fiber->push_null();
        }
        
        return true;
    }
    
    static bool socket_close(MNFiber* fiber)
    {
        MNUserData* userData = fiber->get(1).toUserData();
        if (userData == NULL)
            return false;
        
        TSocket* socket = (TSocket*)userData->getData();
        socket->close();
        return false;
    }
    
    static bool socket_httpRequest(MNFiber* fiber)
    {
        fiber->load_stack(1);
        fiber->tostring();
        tstring method = fiber->get(-1).toString()->ss().str();
        
        fiber->load_stack(2);
        fiber->tostring();
        tstring host = fiber->get(-1).toString()->ss().str();
        
        fiber->load_stack(3);
        fiber->tostring();
        tstring path = fiber->get(-1).toString()->ss().str();
        
        const char* format = "%s %s HTTP/1.1\r\n"
        "ACCEPT_ENCODING: gzip,deflate,sdch\r\n"
        "CONNECTION: keep-alive\r\n"
        "ACCEPT: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "ACCEPT_CHARSET: windows-949,utf-8;q=0.7,*;q=0.3\r\n"
        "USER_AGENT: Mozilla/5.0 (X11; Linux i686) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/13.0.782.24\r\n"
        "ACCEPT_LANGUAGE: ko-KR,ko;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
        "HOST: %s\r\n"
        "\r\n";
        
        char request[512] = {0};
        sprintf(&request[0], format, method.c_str(), path.c_str(), host.c_str());

        fiber->push_string(&request[0]);
        return true;
    }
    
    static bool string_length(MNFiber* fiber)
    {
        MNString* str = fiber->get(1).toString();
        fiber->push_integer((str != NULL)? str->str().length() : 0);
        
        return true;
    }
    
	static void expose(MNFiber* fiber)
	{
        //! common
        {
            make_metaTable(fiber, "table");
            make_metaTable(fiber, "math");
            make_metaTable(fiber, "closure");
            make_metaTable(fiber, "array");
            make_metaTable(fiber, "fiber");
            
            fiber->push_string("print");
            push_closure(fiber, print);
            fiber->store_global();
            
            fiber->push_string("bind");
            push_closure(fiber, bind);
            fiber->store_global();
            
            fiber->push_string("float");
            push_closure(fiber, castFloat);
            fiber->store_global();
            
            fiber->push_string("int");
            push_closure(fiber, castInt);
            fiber->store_global();
            
            fiber->push_string("allocate");
            push_closure(fiber, allocate);
            fiber->store_global();
            
            fiber->push_string("typeof");
            push_closure(fiber, _typeof);
            fiber->store_global();
            
            fiber->push_string("dofile");
            push_closure(fiber, dofile);
            fiber->store_global();
            
            fiber->push_string("dotext");
            push_closure(fiber, dotext);
            fiber->store_global();
            
            fiber->push_string("garbageCollect");
            push_closure(fiber, garbageCollect);
            fiber->store_global();
            
            fiber->push_string("setMeta");
            push_closure(fiber, setMeta);
            fiber->store_global();
            
            fiber->push_string("getMeta");
            push_closure(fiber, getMeta);
            fiber->store_global();
            
            fiber->push_string("delegator");
            push_closure(fiber, delegator);
            fiber->store_global();
            
            fiber->push_string("getSeconds");
            push_closure(fiber, getSeconds);
            fiber->store_global();
        }
        
        //! math
        {
            fiber->push_string("math_sqrt");
            push_closure(fiber, math_sqrt);
            fiber->store_global();
            
            fiber->push_string("math_pow");
            push_closure(fiber, math_pow);
            fiber->store_global();
        }
        
        //! closure
        {
            fiber->push_string("closure_call");
            push_closure(fiber, closure_call);
            fiber->store_global();
            
            fiber->push_string("closure_compileFile");
            push_closure(fiber, closure_compileFile);
            fiber->store_global();
        }
        
        //! fiber
        {
            fiber->push_string("fiber_new");
            push_closure(fiber, fiber_new);
            fiber->store_global();
            
            fiber->push_string("fiber_next");
            push_closure(fiber, fiber_next);
            fiber->store_global();
            
            fiber->push_string("fiber_reset");
            push_closure(fiber, fiber_reset);
            fiber->store_global();
            
            fiber->push_string("fiber_value");
            push_closure(fiber, fiber_value);
            fiber->store_global();
        }
        
        //! array
        {
            fiber->push_string("array_count");
            push_closure(fiber, array_count);
            fiber->store_global();
            
            fiber->push_string("array_add");
            push_closure(fiber, array_add);
            fiber->store_global();
            
            fiber->push_string("array_remove");
            push_closure(fiber, array_remove);
            fiber->store_global();
            
            fiber->push_string("array_clear");
            push_closure(fiber, array_clear);
            fiber->store_global();
            
            fiber->push_string("array_iterate");
            push_closure(fiber, array_iterate);
            fiber->store_global();
        }
        
        //! table
        {
            fiber->push_string("table_has");
            push_closure(fiber, table_has);
            fiber->store_global();
            
            fiber->push_string("table_count");
            push_closure(fiber, table_count);
            fiber->store_global();
            
            fiber->push_string("table_insert");
            push_closure(fiber, table_insert);
            fiber->store_global();
            
            fiber->push_string("table_iterate");
            push_closure(fiber, table_iterate);
            fiber->store_global();
            
            fiber->push_string("table_capacity");
            push_closure(fiber, table_capacity);
            fiber->store_global();
        }
        
		//! opengl
		{
			fiber->push_string("opengl_clear");
			push_closure(fiber, opengl_clear);
			fiber->store_global();

			fiber->push_string("opengl_clearColor");
			push_closure(fiber, opengl_clearColor);
			fiber->store_global();
		}
        
        //! socket
        {
            fiber->push_string("socket_connect");
            push_closure(fiber, socket_connect);
            fiber->store_global();
            
            fiber->push_string("socket_close");
            push_closure(fiber, socket_close);
            fiber->store_global();
            
            fiber->push_string("socket_send");
            push_closure(fiber, socket_send);
            fiber->store_global();
            
            fiber->push_string("socket_read");
            push_closure(fiber, socket_read);
            fiber->store_global();
            
            fiber->push_string("socket_httpRequest");
            push_closure(fiber, socket_httpRequest);
            fiber->store_global();
        }
        
        //! string
        {
            fiber->push_string("string_length");
            push_closure(fiber, string_length);
            fiber->store_global();
        }
	}
};

void MNBasicLib(MNFiber* fiber)
{
	CommonLib::expose(fiber);
}