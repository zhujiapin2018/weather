# Embed CA, certificate & key directly into binarya

idf_component_register(SRCS "wpa2_enterprise_main.c" "user_http_s.c" "lcd.c" "font.c" "tjpgd.c" "decode_image.c"
                    INCLUDE_DIRS "." 
                    EMBED_TXTFILES wpa2_ca.pem wpa2_client.crt wpa2_client.key server_root_cert.pem hzk32s.fon img0.jpg img1.jpg img10.jpg img11.jpg img12.jpg img13.jpg img14.jpg img15.jpg img16.jpg img17.jpg img18.jpg img19.jpg img2.jpg img20.jpg img21.jpg img22.jpg img23.jpg img24.jpg img25.jpg img26.jpg img27.jpg img28.jpg img29.jpg img3.jpg img30.jpg img31.jpg img32.jpg img33.jpg img34.jpg img35.jpg img36.jpg img37.jpg img38.jpg img4.jpg img5.jpg img6.jpg img7.jpg img8.jpg img9.jpg img99.jpg)
#hzk16s.fnt 这个文件放在上一行括号里面，作为嵌入到程序内部的16×16字库
#set(ICONV_ROOT "C:\\Users\\Administrator\\esp\\examples\\wifi\\wpa2_enterprise\\components\\iconv")
#set(Iconv_INCLUDE_DIR "C:\\Users\\Administrator\\esp\\examples\\wifi\\wpa2_enterprise\\components\\iconv\\include")
#set(Iconv_LIBRARY "C:\\Users\\Administrator\\esp\\examples\\wifi\\wpa2_enterprise\\components\\iconv\\lib")
# 找到iconv特定版本,REQUIRED强调必须找到该模块，否则编译失败；

# COMPONENTS后面罗列一组子模块
#find_package(iconv REQUIRED)

# 验证是否发现iconv
#if(Iconv_FOUND)
#message ("iconv found")
#else()
#message (FATAL_ERROR "Cannot find iconv")
#endif()

#add_executable(wpa2-enterprise wpa2_enterprise_main.c)

# 链接指定的boost库文件和头文件。
#target_include_directories( ${project}
#PRIVATE
#${Iconv_INCLUDES}
#)
#target_link_libraries( ${project}
#PRIVATE
#${Iconv_LIBRARIES}
#)              