
#pragma once

#include <imgui/imgui.h>

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
    void Combo(literal label, E& val, ImGuiComboFlags flags = 0) {
		
		if(BeginCombo(label, enum_name(val), flags)) {

            enum_iterate<E>([&val](E cval, literal cname) {
				bool selected = val == cval;
				if(Selectable(cname, selected)) val = cval;
				if(selected) SetItemDefaultFocus();
            });

			EndCombo();
		}
    }

	template<typename V, typename A> 
    void Combo(literal label, map<astring<A>,V> options, V& val, ImGuiComboFlags flags = 0) {

        astring<A> preview;
        for(auto& opt : options) {
            if(opt.value == val) preview = opt.key;
        }

        if(BeginCombo(label, preview, flags)) {
            for(auto& opt : options) {
                bool selected = val == opt.value;
                if(Selectable(opt.key, selected)) val = opt.value;
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
					View(val[i]);
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

    template<typename S>
    struct gui_type<S, Type_Type::ptr_> {
        static void view(S val, bool open) {
            if(val) gui_type<typename No_Ptr<S>::type, Type_Info<typename No_Ptr<S>::type>::type>::view(*val, open);
            else Text("(null)");
        }
        static void edit(literal label, S& val, bool open) {
            if(val) gui_type<typename No_Ptr<S>::type, Type_Info<typename No_Ptr<S>::type>::type>::edit(label, *val, open);
            else Text("(null)");
        }
    };
    template<>
    struct gui_type<char const*, Type_Type::ptr_> {
        static void view(char const* val, bool open) {
            Text(val);
        }
    };
    template<>
    struct gui_type<decltype(nullptr), Type_Type::ptr_> {
        static void view(char const* val, bool open) {}
        static void edit(literal label, char* val, bool open) {}
    };
    template<>
    struct gui_type<char*, Type_Type::ptr_> {
        static void view(char* val, bool open) {
            Text(val);
        }
        static void edit(literal label, char* val, bool open) {
            InputScalar(label, ImGuiDataType_S8, val);
        }
    };

    template<typename S>
    struct gui_type<S, Type_Type::fptr_> {
        static void view(S val, bool open) {
            if(val) Text(Type_Info<S>::name);
            else Text("(null)");
        }
        static void edit(literal label, S& val, bool open) {
            if(val) Text(Type_Info<S>::name);
            else Text("(null)");
        }
    };    

    template<typename S>
    struct gui_type<S, Type_Type::enum_> {
        static void view(S val, bool open) {
            Text(scratch_format("%", val));
        }
        static void edit(literal label, S& val, bool open) {
            Combo(label, val);
        }
    };

    template<typename S, typename H, typename T>
    struct gui_field {
        using member = typename H::type;
        static void view(S val, bool open) {
            Text(H::name); SameLine();
            gui_type<member, Type_Info<member>::type>::view(*(member*)((char*)&val + H::offset), open);
            gui_field<S, typename T::head, typename T::tail>::view(val, open);
        }
        static void edit(S& val, bool open) {
            gui_type<member, Type_Info<member>::type>::edit(H::name, *(member*)((char*)&val + H::offset), open);
            gui_field<S, typename T::head, typename T::tail>::edit(val, open);
        }
    };
    template<typename S, typename H>
    struct gui_field<S, H, void> {
        using member = typename H::type;
        static void view(S val, bool open) {
            Text(H::name); SameLine();
            gui_type<member, Type_Info<member>::type>::view(*(member*)((char*)&val + H::offset), open);
        }
        static void edit(S& val, bool open) {
            gui_type<member, Type_Info<member>::type>::edit(H::name, *(member*)((char*)&val + H::offset), open);
        }
    };
    template<typename S>
    struct gui_field<S, void, void> {
        static void view(S val, bool open) {}
        static void edit(S& val, bool open) {}
    };

    template<typename S>
    struct gui_type<S, Type_Type::record_> {
        using members = typename Type_Info<S>::members;
        static void view(S val, bool open) {
            if(TreeNodeEx(Type_Info<S>::name, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
                gui_field<S, typename members::head, typename members::tail>::view(val, open);
                TreePop();
            }
        }
        static void edit(literal label, S& val, bool open) {
            if(TreeNodeEx(label, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
                gui_field<S, typename members::head, typename members::tail>::edit(val, open);
                TreePop();
            }
        }
    };

    template<typename A>
    struct gui_type<astring<A>, Type_Type::string_> {
        static void view(astring<A> val, bool open) {
            Text(val);
        }
        static void edit(literal label, astring<A>& val, bool open) {
            InputText(label, val);
        }
    };

    template<typename T, typename A>
    struct gui_type<vec<T,A>, Type_Type::record_> {
        static void view(vec<T,A> val, bool open) {
			if(TreeNodeEx(Type_Info<T>::name, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				for(usize i = 0; i < val.size; i++) {
					PushID(i);
					View(val[i]);
					PopID();
				}
				TreePop();
			}
        }
        static void edit(literal label, vec<T,A>& val, bool open) {
			if(TreeNodeEx(label, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				for(usize i = 0; i < val.size; i++) {
					PushID(i);
					Edit("", val[i]);
					PopID();
				}
                if(SmallButton("+")) {
                    val.push(T());
                }
                SameLine();
                if(SmallButton("-") && !val.empty()) {
                    val.pop();
                }
				TreePop();
			}
        }
    };

    template<typename T, typename A>
    struct gui_type<stack<T,A>, Type_Type::record_> {
        static void view(stack<T,A> val, bool open) {
            gui_type<vec<T,A>, Type_Type::record_>::view(val.data, open);
        }
        static void edit(literal label, stack<T,A>& val, bool open) {
            gui_type<vec<T,A>, Type_Type::record_>::edit(label, val.data, open);
        }
    };

    template<typename T, typename A>
    struct gui_type<queue<T,A>, Type_Type::record_> {
        static void view(queue<T,A> val, bool open) {
			if(TreeNodeEx(Type_Info<T>::name, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				usize i = 0;
                for(auto& v : val) {
					PushID(i++);
					View(v);
					PopID();
				}
				TreePop();
			}
        }
        static void edit(literal label, queue<T,A>& val, bool open) {
			if(TreeNodeEx(Type_Info<T>::name, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				usize i = 0;
                for(auto& v : val) {
					PushID(i++);
					Edit(label, v);
					PopID();
				}
                if(SmallButton("+")) {
                    val.push(T());
                }
                SameLine();
                if(SmallButton("-") && !val.empty()) {
                    val.pop();
                }
				TreePop();
			}
        }
    };

    template<typename T, typename A>
    struct gui_type<heap<T,A>, Type_Type::record_> {
        static void view(heap<T,A> val, bool open) {
			if(TreeNodeEx(Type_Info<T>::name, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				usize i = 0;
                for(auto& v : val) {
					PushID(i++);
					View(v);
					PopID();
				}
				TreePop();
			}
        }
        // NOTE(max): could add heap editing with reheap on edit, but don't need it for the forseeable future.
    };

    template<typename K, typename V, typename A, Hash<K> H>
    struct gui_type<map<K,V,A,H>, Type_Type::record_> {
        static void view(map<K,V,A,H> val, bool open) {
			if(TreeNodeEx(Type_Info<map<K,V,A,H>>::name, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				usize i = 0;
				Columns(2);
                for(auto& v : val) {
					PushID(i++);
					View(v.key);
                    PopID();
                    NextColumn();
                    PushID(i++);
					View(v.value);
					PopID();
                    NextColumn();
				}
				TreePop();
			}
        }
        static void edit(literal label, map<K,V,A,H>& val, bool open) {
			if(TreeNodeEx(Type_Info<map<K,V,A,H>>::name, open ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				usize i = 0;
				Columns(2);
                for(auto& v : val) {
					PushID(i++);
					Edit("", v.key);
                    PopID();
                    NextColumn();
                    PushID(i++);
					Edit("", v.value);
					PopID();
                    NextColumn();
				}
                Columns();
				TreePop();
			}
        }
    };

    template<typename T, usize N>
    struct gui_type<vect<T,N>, Type_Type::record_> {
        static void view(vect<T,N> val, bool open) {
			gui_type<T[N], Type_Type::array_>::view(val.data, open);
        }
        static void edit(literal label, vect<T,N>& val, bool open) {
			gui_type<T[N], Type_Type::array_>::edit(label, val.data, open);
        }
    };

    template<>
    struct gui_type<v2, Type_Type::record_> {
        static void view(v2 val, bool open) {
			Text("[%f,%f]", val.x, val.y);
        }
        static void edit(literal label, v2& val, bool open) {
			InputFloat2(label, val.data);
        }
    };

    template<>
    struct gui_type<v3, Type_Type::record_> {
        static void view(v3 val, bool open) {
			Text("[%f,%f,%f]", val.x, val.y, val.z);
        }
        static void edit(literal label, v3& val, bool open) {
			InputFloat3(label, val.data);
        }
    };

    template<>
    struct gui_type<v4, Type_Type::record_> {
        static void view(v4 val, bool open) {
			Text("[%f,%f,%f,%f]", val.x, val.y, val.z, val.w);
        }
        static void edit(literal label, v4& val, bool open) {
			InputFloat4(label, val.data);
        }
    };

    template<>
    struct gui_type<v2i, Type_Type::record_> {
        static void view(v2i val, bool open) {
			Text("[%d,%d]", val.x, val.y);
        }
        static void edit(literal label, v2i& val, bool open) {
			InputInt2(label, val.data);
        }
    };

    template<>
    struct gui_type<v3i, Type_Type::record_> {
        static void view(v3i val, bool open) {
			Text("[%d,%d,%d]", val.x, val.y, val.z);
        }
        static void edit(literal label, v3i& val, bool open) {
			InputInt3(label, val.data);
        }
    };

    template<>
    struct gui_type<v4i, Type_Type::record_> {
        static void view(v4i val, bool open) {
			Text("[%d,%d,%d,%d]", val.x, val.y, val.z, val.w);
        }
        static void edit(literal label, v4i& val, bool open) {
			InputInt4(label, val.data);
        }
    };
}


