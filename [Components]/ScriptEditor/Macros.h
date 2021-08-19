#pragma once

#define ImplPropertyGetSimple(Type, BackingStoreName) virtual Type get() { return BackingStoreName; }
#define ImplPropertySetSimple(Type, BackingStoreName) virtual void set(Type v) { BackingStoreName = v; }
#define ImplPropertySetInvalid(Type) virtual void set(Type v) { throw gcnew InvalidOperationException("Getter-only property!"); }

#define ImplPropertySimple(Type, Name, BackingStoreName) property Type Name { virtual Type get() { return BackingStoreName; } virtual void set(Type v) { BackingStoreName = v; } }
#define ImplPropertyGetOnly(Type, Name, BackingStoreName) property Type Name { virtual Type get() { return BackingStoreName; } virtual void set(Type v) { throw gcnew InvalidOperationException("Getter-only property!"); } }
#define ImplPropertyWithAccessors(Type, Name) property Type Name { virtual Type get() { return Getter##Name(); } virtual void set(Type v) { Setter##Name(v); } }
#define ImplPropertyGetOnlyAccessor(Type, Name) property Type Name { virtual Type get() { return Getter##Name(); } virtual void set(Type v) { throw gcnew InvalidOperationException("Getter-only property!"); } }
