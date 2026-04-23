# CG-FinalProject-Nazer
Final Project for Computer Graphics course - 3D Maze Game using OpenGL.
# 3D Maze Runner - Final Project 🎮

مشروع لعبة متاهة ثلاثية الأبعاد تم تطويرها باستخدام لغة C++ ومكتبة OpenGL. تعتمد اللعبة على تجميع العملات الذهبية للوصول إلى شرط الفوز، مع تطبيق تقنيات إضاءة وضباب (Fog) وتصادم (Collision Detection).

## (Features)
* Hierarchical Modeling: بناء شخصية اللاعب بشكل هرمي (رأس، جذع، أطراف).
* State Machine: تغيير حالة اللاعب بصرياً عند الحركة (تغيير اللون بين الأزرق والأخضر).
* Camera System: كاميرا مطاردة (Third-person camera) تتبع اللاعب بسلاسة باستخدام glm::mix.
* Environment: أرضية وجدران مكسوة بالصور (Textures) مع نظام ضباب واقعي يعتمد على المسافة.
* Gameplay Logic: نظام جمع كوينز (6 كوينز موزعة)، عداد نقاط، وعداد وقت يظهر في عنوان النافذة.

## (Controls)
* W / S / A / D: للحركة في كافة الاتجاهات داخل المتاهة.
* Mouse: لتحريك الكاميرا والنظر حول اللاعب.
* ESC: للخروج من اللعبة.

##  (Dependencies)
تعتمد اللعبة على المكتبات التالية (مرفقة ضمن إعدادات المشروع):
* GLFW & GLEW: لإدارة النوافذ ومعالجة أوامر الـ Graphics.
* GLM: للعمليات الرياضية والمصفوفات.
* stb_image: لتحميل وإدارة الصور (Textures).
إعداد الطالب: Nazer
