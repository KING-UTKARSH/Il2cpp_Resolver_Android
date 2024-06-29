#pragma once

namespace Unity
{
	struct System_String : il2cppObject
	{
		int m_iLength;
		wchar_t m_wString[1024];

		void Clear()
		{
			if (!this) return;

			memset(m_wString, 0, static_cast<size_t>(m_iLength) * 2);
			m_iLength = 0;
		}

		std::string ToString()
		{
			if (!this || m_wString == nullptr || m_iLength == 0)
                return "";
			
			std::string retStr;
			mbstate_t state = mbstate_t();
            char buf[MB_CUR_MAX];
            
            for (size_t i = 0; i < m_iLength; ++i)
            {
                size_t ret = wcrtomb(buf, m_wString[i], &state);
                if (ret == (size_t)-1)
                {
                    return "";
                }
                retStr.append(buf, ret);
            }
			return retStr;
		}
	};
}