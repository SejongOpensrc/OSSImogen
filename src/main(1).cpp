#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include <math.h>
#include <vector>
#include "Nodes.h"
#include "NodesDelegate.h"
#include "Evaluation.h"
#include "Imogen.h"

// ------------------------------------- �Ӹ� �ּ� ---------------------------------

// ���� �̸� : Imogen.exe
// �ۼ��� : CedricGuillemet (https://github.com/junhan-kim/Imogen)
// ���� : 2D �ؽ��ĸ� ��� ��� GUI�� �����ϰ� �����ϱ� ���� ���α׷�.
// ���� ���� : -
// ���� ó�� : -
// �̷� ���� : main.cpp�� �ٴ��� �ּ� �� �Ӹ��ּ� �߰� (������, 2018-10-31) 



int Log(const char *szFormat, ...) // �������ڸ� �޾� �α׸� ����ϴ� �Լ�
{
	va_list ptr_arg;
	va_start(ptr_arg, szFormat); // ptr_arg���� ���������� ù��° ������ �ּҸ� �˷���.

	char buf[1024];
	vsprintf(buf, szFormat, ptr_arg); // �������ڵ��� buf�� ����

	static FILE *fp = fopen("log.txt", "wt"); // log.txt�� write���� ���� fp�� ���������͸� ����
	if (fp) // ������ ������
	{
		fprintf(fp, buf); // buf�� �ִ� ���� fp�� ����Ű�� ���Ͽ� ��
		fflush(fp);
	}
	DebugLogText(buf); // buf�� �ִ� ������ �����
	va_end(ptr_arg);
	return 0;
}

int main(int, char**)
{
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) // SDL�� Ÿ�̸� �ý��۰� ����� �ý����� �ʱ�ȭ��
	{
		printf("Error: %s\n", SDL_GetError()); // �ʱ�ȭ�� ������ ������ Error �ڵ� ��� �� ���α׷� ����
		return -1;
	}

	// Decide GL+GLSL versions
#if __APPLE__ // ������ ���
	// GL 3.2 Core + GLSL 150
	const char* glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else // �� ���� ȯ���� ���
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";                                     // glsl ���� ���
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);                                  // SDL ���� �÷��� : 0(�⺻��)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // OpenGL �ھ� �������� ���� : SDL_GL_CONTEXT_PROFILE_CORE(�⺻��)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);                          // ���ؽ�Ʈ �ֿ� ���� : 3
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);                          // ���ؽ�Ʈ �� ���� : 0
#endif
	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);     // ���� ���۸� ��� : 1(���)
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);      // ���� ����(z ����)�� �ִ� ��Ʈ �� : 24
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);     // ���ٽ� ���� �ּ� ��Ʈ �� : 8
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);          // ���� ���÷��� ��带 current�� ����
	SDL_Window* window = SDL_CreateWindow("Imogen 0.1.1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
												     // ������ â ���� (���� : Imogen 0.1.1
                                                     //                 ��ġ : ȭ�� �߾�
                                                     //                 �ػ� : 1280 x 720
                                                     //                 �÷��� : OpenGL ����, ������ ���� ����, �ִ� ũ��� ����)
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);	// OpenGL ���ؽ�Ʈ �����ϰ� gl_context�� ����
	SDL_GL_SetSwapInterval(1);									// Enable vsync

							   // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)    // GL3W�� ���ǵ� ���
	bool err = gl3wInit() != 0;               
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)  // GLEW�� ���ǵ� ���
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)  // GLAD�� ���ǵ� ���
	bool err = gladLoadGL() == 0;
#endif
	if (err) // ���� ����üũ���� ������ �߻��� ���, ���� ��� �� ����
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();                             // ���� üũ
	ImGui::CreateContext();                           // Imgui ���ؽ�Ʈ ����
	ImGuiIO& io = ImGui::GetIO(); (void)io;           // ����� ���� ����
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context); // imgui�� ���� �����쿡 ���ε� �� �ʱ�ȭ
	ImGui_ImplOpenGL3_Init(glsl_version);             // OpenGL �ʱ�ȭ

	// Setup style
	ImGui::StyleColorsDark();                         // �׸��÷��� ���������� ����

	static const char* libraryFilename = "library.dat";
	Library library;                                     
	LoadLib(&library, libraryFilename);                  // ���̺귯�� �ε� �� �����̸� ����
	
	Evaluation evaluation;
	Imogen imogen;
	
	evaluation.SetEvaluationGLSL(imogen.shaderFileNames);  // glsl�� ���� ���̴��� �̿��Ͽ� ���Ŀ� ����� ���۾��� �ʱ�ȭ��
	evaluation.LoadEquiRect("studio017PoT.png");           // �������� ������ Rect�� �ʱ�ȭ

	TileNodeEditGraphDelegate nodeGraphDelegate(evaluation);

	// Main loop
	bool done = false;
	while (!done) // �����̺�Ʈ ������ ���� ����
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) // �̺�Ʈ ����
		{
			ImGui_ImplSDL2_ProcessEvent(&event);  // �̺�Ʈ�� imgui���� �ν�
			if (event.type == SDL_QUIT) // �������� ���� �̺�Ʈ�� ���� ���
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				                        // ������ â���� �̺�Ʈ�� CLOSE ����� ���� ���
				done = true;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();       // OpenGL ������ ����
		ImGui_ImplSDL2_NewFrame(window);    // SDL ������ ����
		ImGui::NewFrame();                  // imgui ������ ����

		imogen.Show(library, nodeGraphDelegate, evaluation);  // ���α׷��� ��ü ������ Show (������ �۾����� ���ΰ�ħ ���)

		evaluation.RunEvaluation();  // �����α׷��� ���� ��� �۾��� �Ѱ��ϴ� �Լ�. 
		                             //   �������� ������ �Է� ����(�̺�Ʈ)���� ���������� ���Ͽ�, ���� ������ ��ȭ��Ŵ

		// render everything
		glClearColor(0.45f, 0.4f, 0.4f, 1.f);     // RGBA ���·� �÷����۸� ����� �뵵�� �� ����
		glClear(GL_COLOR_BUFFER_BIT);             // �÷� ���� Ŭ����
		ImGui::Render();                          // ������

		SDL_GL_MakeCurrent(window, gl_context);                               // OpenGL�� �������ϱ� ���� ���ؽ�Ʈ ����
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);       // ����Ʈ�� ǥ�� ������ ���� ( 0, 0 ������ io�� ���÷��� ũ�� ������ŭ )
		glClearColor(0.,0.,0.,0.);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());               // Imgui�� ������
		SDL_GL_SwapWindow(window);                                            // â�� OpenGL �������� ���� ������Ʈ
	}
	
	SaveLib(&library, libraryFilename);       // ���̺귯�� ����
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();             // OpenGL ����
	ImGui_ImplSDL2_Shutdown();                // SDL ����
	ImGui::DestroyContext();                  // imgui ���ؽ�Ʈ ����

	SDL_GL_DeleteContext(gl_context);         // SDL ���ؽ�Ʈ ����
	SDL_DestroyWindow(window);                // ������ â ����
	SDL_Quit();                               // SDL ����

	return 0;
}
