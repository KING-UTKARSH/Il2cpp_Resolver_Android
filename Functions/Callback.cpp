#include "../Includes.hpp"

namespace IL2CPP
{
	namespace Callback
	{
		struct _VFuncCallback
		{
			std::vector<void*> m_vFunctions;

			void** m_pVTable = nullptr;
			void* m_pOriginal = nullptr;
		};

		namespace OnUpdate
		{
			_VFuncCallback Data;

			void Add(void* m_pFunction) { Data.m_vFunctions.emplace_back(m_pFunction); }

			void __fastcall Caller(void* rcx)
			{
				for (void* m_pFunction : Data.m_vFunctions)
					reinterpret_cast<void(*)()>(m_pFunction)();

				reinterpret_cast<void(__fastcall*)(void*)>(Data.m_pOriginal)(rcx);
			}
		}

		namespace OnLateUpdate
		{
			_VFuncCallback Data;

			void Add(void* m_pFunction) { Data.m_vFunctions.emplace_back(m_pFunction); }

			void __fastcall Caller(void* rcx)
			{
				for (void* m_pFunction : Data.m_vFunctions)
					reinterpret_cast<void(*)()>(m_pFunction)();

				reinterpret_cast<void(__fastcall*)(void*)>(Data.m_pOriginal)(rcx);
			}
		}

		void Replace_VFunc(void** m_pVTableFunc, void* m_pCaller, void** m_pCallbackOriginal)
		{ // Researched code, I don't know if it works properly as VirtualProtect from WIN API
            long pageSize = sysconf(_SC_PAGESIZE);
            
            uintptr_t pageStart = ((uintptr_t)m_pVTableFunc) & ~(pageSize - 1);
        
            if (mprotect((void*)pageStart, pageSize, PROT_READ | PROT_WRITE) == 0)
            {
                if (m_pCallbackOriginal)
                    *m_pCallbackOriginal = *m_pVTableFunc;
                *m_pVTableFunc = m_pCaller;
        
                mprotect((void*)pageStart, pageSize, PROT_READ | PROT_EXEC);
            }
		}

		void Initialize()
		{
			void* m_pThread = IL2CPP::Thread::Attach(IL2CPP::Domain::Get());

			// Fetch
			{
				void** m_pMonoBehaviourVTable = *reinterpret_cast<void***>(IL2CPP::Helper::GetMonoBehaviour()->m_CachedPtr);
				if (m_pMonoBehaviourVTable) // x86: darkness my old friend
				{
#ifdef __aarch64__ // ARM 64-bit
					OnUpdate::Data.m_pVTable		= VFunc::Find_ASM(m_pMonoBehaviourVTable, 99, { 0x02, 0x00, 0x80, 0xD2, 0x00, 0x00, 0x00, 0x14 }); // mov x2, #0 | b
					OnLateUpdate::Data.m_pVTable	= VFunc::Find_ASM(m_pMonoBehaviourVTable, 99, { 0x22, 0x00, 0x80, 0xD2, 0x00, 0x00, 0x00, 0x14 }); // mov x2, #1 | b
#elif defined(__arm__) // ARM 32-bit
					OnUpdate::Data.m_pVTable		= VFunc::Find_ASM(m_pMonoBehaviourVTable, 99, { 0x00, 0x20, 0xA0, 0xE3, 0x00, 0x00, 0x00, 0xEA }); // mov r2, #0 | b
					OnLateUpdate::Data.m_pVTable	= VFunc::Find_ASM(m_pMonoBehaviourVTable, 99, { 0x01, 0x20, 0xA0, 0xE3, 0x00, 0x00, 0x00, 0xEA }); // mov r2, #1 | b
#endif
				}
			}

			IL2CPP::Thread::Detach(m_pThread);

			// Replace
			{
				Replace_VFunc(OnUpdate::Data.m_pVTable,		(void*)OnUpdate::Caller,		&OnUpdate::Data.m_pOriginal);
				Replace_VFunc(OnLateUpdate::Data.m_pVTable,	(void*)OnLateUpdate::Caller,	&OnLateUpdate::Data.m_pOriginal);
			}
		}

		void Uninitialize()
		{
			Replace_VFunc(OnUpdate::Data.m_pVTable,		OnUpdate::Data.m_pOriginal,		nullptr);
			Replace_VFunc(OnLateUpdate::Data.m_pVTable,	OnLateUpdate::Data.m_pOriginal,	nullptr);
		}
	}
}