
#pragma once

#ifndef RUNNING_META
#include <imgui/imgui.h>
#endif

#include "lib/lib.h"

namespace ImGui {

    template<typename A>
	bool InputText(literal label, astring<A> buf, ImGuiInputTextFlags flags = 0, 
                   ImGuiTextEditCallback callback = null, void* user_data = null) {
        
        return InputText(label.c_str, buf.c_str, buf.cap, flags, callback, user_data);
    }

    inline void Text(literal text) {
        TextUnformatted(text.c_str, text.c_str + text.len - 1);
    }

	template<typename E> 
    void Combo(string label, E* val, ImGuiComboFlags flags = 0) {
		
        using info = Type_Info<E>;

		if(BeginCombo(label, enum_name(*val), flags)) {

            enum_iterate<E>([val](E cval, literal cname) {
				bool selected = *val == cval;
				if(Selectable(cname, selected)) *val = cval;
				if(selected) SetItemDefaultFocus();
            });

			EndCombo();
		}
    }

	template<typename V, typename A> 
    void Combo(literal label, map<astring<A>,V> options, V* val, ImGuiComboFlags flags = 0) {

        astring<A> preview;
        for(auto& opt : options) {
            if(opt.value == *val) preview = opt.key;
        }

        if(BeginCombo(label, preview, flags)) {
            for(auto& opt : options) {
                bool selected = *val == opt.value;
                if(Selectable(opt.key, selected)) *val = opt.value;
                if(selected) SetItemDefaultFocus();
            }
        }
    }

    template<typename E, Type_Type T>
    struct gui_type {};

	template<typename S>
    void View(S val, bool open = false) {
        gui_type<S, Type_Info<S>::type>::view(val, open);
    }
	template<typename S, usize N>
    void view(S(&val)[N], bool open = false) {
        gui_type<S[N], Type_Info<S[N]>::type>::view(val, open);
    }

	template<typename S> 
    void Edit(literal label, S& val, bool open = false) {
        gui_type<S, Type_Info<S>::type>::edit(label, val, open);
    }
	template<typename S, usize N>
    void Edit(literal label, S(&val)[N], bool open = false) {
        gui_type<S[N], Type_Info<S[N]>::type>::edit(label, val, open);
    }

    template<typename S>
    struct gui_type<S, Type_Type::int_> {
        static void view(S val, bool open) {
            if(Type_Info<S>::sgn) {
                switch(Type_Info<S>::size) {
                case 1: Text("%hhd", val); break;
                case 2: Text("%hd", val); break;
                case 4: Text("%d", val); break;
                case 8: Text("%lld", val); break;
                }
            } else {
                switch(Type_Info<S>::size) {
                case 1: Text("%hhu", val); break;
                case 2: Text("%hu", val); break;
                case 4: Text("%u", val); break;
                case 8: Text("%llu", val); break;
                }
            }
        }
        static void edit(literal label, S& val, bool open) {
            if(Type_Info<S>::sgn) {
                switch(Type_Info<S>::size) {
                case 1: InputScalar(label, ImGuiDataType_S8, &val); break;
                case 2: InputScalar(label, ImGuiDataType_S16, &val); break;
                case 4: InputScalar(label, ImGuiDataType_S32, &val); break;
                case 8: InputScalar(label, ImGuiDataType_S64, &val); break;
                }
            } else {
                switch(Type_Info<S>::size) {
                case 1: InputScalar(label, ImGuiDataType_U8, &val); break;
                case 2: InputScalar(label, ImGuiDataType_U16, &val); break;
                case 4: InputScalar(label, ImGuiDataType_U32, &val); break;
                case 8: InputScalar(label, ImGuiDataType_U64, &val); break;
                }
            }
        }
    };

    template<typename S>
    struct gui_type<S, Type_Type::float_> {
        static void view(S val, bool open) {
            Text("%f", (f64)val);
        }
        static void edit(literal label, S& val, bool open) {
            if constexpr(Type_Info<S>::size == 4) InputScalar(label, ImGuiDataType_Float, &val);
            else InputScalar(label, ImGuiDataType_Double, &val);
        }
    };

    template<typename S>
    struct gui_type<S, Type_Type::bool_> {
        static void view(S val, bool open) {
            Text(val ? "true" : "false");
        }
        static void edit(literal label, S& val, bool open) {
            Checkbox(label, &val);
        }
    };

    template<typename S>
    struct gui_type<S, Type_Type::char_> {
        static_assert(sizeof(S) == 1);
        static void view(S val, bool open) {
            Text("%c", val);
        }
        static void edit(literal label, S& val, bool open) {
            InputScalar(label, ImGuiDataType_S8, &val);
        }
    };

    template<>
    struct norefl gui_type<char const*, Type_Type::ptr_> {
        static void view(char const* val, bool open) {
            Text(val);
        }
    };
    template<>
    struct norefl gui_type<char*, Type_Type::ptr_> {
        static void view(char* val, bool open) {
            Text(val);
        }
        static void edit(literal label, char* val, bool open) {
            InputScalar(label, ImGuiDataType_S8, val);
        }
    };

    template<typename S>
    struct gui_type<S, Type_Type::array_> {
        using underlying = typename Type_Info<S>::underlying;
        using gui_underlying = gui_type<underlying, Type_Info<underlying>::type>;
        static constexpr usize len = Type_Info<S>::len;

        static void view(S val, bool open) {
            if constexpr(Type_Info<underlying>::type == Type_Type::char_) {
                Text(val);
                return;
            }
			if(TreeNodeEx(Type_Info<S>::name, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				for(usize i = 0; i < len; i++) {
					PushID(i);
					View("", &val[i]);
					PopID();
				}
				TreePop();
			}
        }
        static void edit(literal label, S& val, bool open) {
            if constexpr(Type_Info<underlying>::type == Type_Type::char_) {
                InputText(label, val, len);
                return;
            }
			if(TreeNodeEx(label, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				for(usize i = 0; i < len; i++) {
					PushID(i);
					Edit("", val[i]);
					PopID();
				}
				TreePop();
			}
        }
    };

    // TODO(max):
    // ptr_
    // decltype(nullptr)
    // fptr_
    // enum_
    // record_
    // astring
    // vec
    // heap
    // map
    // stack
    // queue
    // math types
}


