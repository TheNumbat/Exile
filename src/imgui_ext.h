
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
    void Combo(literal label, map<astring<A>,V> options, V* val, ImGuiComboFlags flags = 0);

	template<typename S>
    void View(literal label, S val, bool open = false);
	template<typename S> 
    void Edit(literal label, S* val, bool open = false);
}
