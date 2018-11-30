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

// ------------------------------------- 머리 주석 ---------------------------------

// 파일 이름 : Imogen.exe
// 작성자 : CedricGuillemet (https://github.com/junhan-kim/Imogen)
// 목적 : 2D 텍스쳐를 노드 기반 GUI로 간편하게 생성하기 위한 프로그램.
// 제한 사항 : -
// 오류 처리 : -
// 이력 사항 : main.cpp의 줄단위 주석 및 머리주석 추가 (김준한, 2018-10-31) 



int Log(const char *szFormat, ...) // 가변인자를 받아 로그를 기록하는 함수
{
	va_list ptr_arg;
	va_start(ptr_arg, szFormat); // ptr_arg에게 가변인자중 첫번째 인자의 주소를 알려줌.

	char buf[1024];
	vsprintf(buf, szFormat, ptr_arg); // 가변인자들을 buf에 받음

	static FILE *fp = fopen("log.txt", "wt"); // log.txt를 write모드로 열어 fp에 파일포인터를 저장
	if (fp) // 문제가 없으면
	{
		fprintf(fp, buf); // buf에 있는 값을 fp가 가리키는 파일에 씀
		fflush(fp);
	}
	DebugLogText(buf); // buf에 있는 값으로 디버깅
	va_end(ptr_arg);
	return 0;
}

int main(int, char**)
{
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) // SDL의 타이머 시스템과 오디오 시스템을 초기화함
	{
		printf("Error: %s\n", SDL_GetError()); // 초기화에 문제가 있을시 Error 코드 출력 및 프로그램 종료
		return -1;
	}

	// Decide GL+GLSL versions
#if __APPLE__ // 애플인 경우
	// GL 3.2 Core + GLSL 150
	const char* glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else // 그 외의 환경인 경우
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";                                     // glsl 버전 기록
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);                                  // SDL 설정 플래그 : 0(기본값)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // OpenGL 코어 프로파일 설정 : SDL_GL_CONTEXT_PROFILE_CORE(기본값)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);                          // 컨텍스트 주요 버전 : 3
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);                          // 컨텍스트 부 버전 : 0
#endif
	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);     // 이중 버퍼링 사용 : 1(사용)
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);      // 깊이 버퍼(z 버퍼)의 최대 비트 수 : 24
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);     // 스텐실 버퍼 최소 비트 수 : 8
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);          // 현재 디스플레이 모드를 current에 저장
	SDL_Window* window = SDL_CreateWindow("Imogen 0.1.1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
												     // 윈도우 창 생성 (제목 : Imogen 0.1.1
                                                     //                 위치 : 화면 중앙
                                                     //                 해상도 : 1280 x 720
                                                     //                 플래그 : OpenGL 형식, 사이즈 조정 가능, 최대 크기로 시작)
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);	// OpenGL 컨텍스트 생성하고 gl_context에 전달
	SDL_GL_SetSwapInterval(1);									// Enable vsync

							   // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)    // GL3W가 정의된 경우
	bool err = gl3wInit() != 0;               
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)  // GLEW가 정의된 경우
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)  // GLAD가 정의된 경우
	bool err = gladLoadGL() == 0;
#endif
	if (err) // 위의 에러체크에서 에러가 발생한 경우, 에러 출력 후 종료
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();                             // 버전 체크
	ImGui::CreateContext();                           // Imgui 컨텍스트 생성
	ImGuiIO& io = ImGui::GetIO(); (void)io;           // 입출력 정보 저장
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context); // imgui를 현재 윈도우에 바인딩 및 초기화
	ImGui_ImplOpenGL3_Init(glsl_version);             // OpenGL 초기화

	// Setup style
	ImGui::StyleColorsDark();                         // 테마컬러를 검은색으로 설정

	static const char* libraryFilename = "library.dat";
	Library library;                                     
	LoadLib(&library, libraryFilename);                  // 라이브러리 로드 및 파일이름 설정
	
	Evaluation evaluation;
	Imogen imogen;
	
	evaluation.SetEvaluationGLSL(imogen.shaderFileNames);  // glsl에 사용될 셰이더를 이용하여 이후에 수행될 평가작업을 초기화함
	evaluation.LoadEquiRect("studio017PoT.png");           // 렌더링을 진행할 Rect를 초기화

	TileNodeEditGraphDelegate nodeGraphDelegate(evaluation);

	// Main loop
	bool done = false;
	while (!done) // 종료이벤트 들어오면 루프 나감
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) // 이벤트 폴링
		{
			ImGui_ImplSDL2_ProcessEvent(&event);  // 이벤트를 imgui에서 인식
			if (event.type == SDL_QUIT) // 보편적인 종료 이벤트가 들어온 경우
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				                        // 윈도우 창에서 이벤트로 CLOSE 명령이 들어온 경우
				done = true;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();       // OpenGL 프레임 생성
		ImGui_ImplSDL2_NewFrame(window);    // SDL 프레임 생성
		ImGui::NewFrame();                  // imgui 프레임 생성

		imogen.Show(library, nodeGraphDelegate, evaluation);  // 프로그램의 전체 내용을 Show (일종의 작업단위 새로고침 기능)

		evaluation.RunEvaluation();  // ★프로그램의 거의 모든 작업을 총괄하는 함수. 
		                             //   이제까지 누적된 입력 내용(이벤트)들을 순차적으로 평가하여, 내부 값들을 변화시킴

		// render everything
		glClearColor(0.45f, 0.4f, 0.4f, 1.f);     // RGBA 형태로 컬러버퍼를 지우는 용도의 값 지정
		glClear(GL_COLOR_BUFFER_BIT);             // 컬러 버퍼 클리어
		ImGui::Render();                          // 렌더링

		SDL_GL_MakeCurrent(window, gl_context);                               // OpenGL로 렌더링하기 위한 컨텍스트 지정
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);       // 뷰포트의 표시 영역을 지정 ( 0, 0 지점에 io의 디스플레이 크기 영역만큼 )
		glClearColor(0.,0.,0.,0.);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());               // Imgui로 렌더링
		SDL_GL_SwapWindow(window);                                            // 창을 OpenGL 렌더링을 통해 업데이트
	}
	
	SaveLib(&library, libraryFilename);       // 라이브러리 저장
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();             // OpenGL 종료
	ImGui_ImplSDL2_Shutdown();                // SDL 종료
	ImGui::DestroyContext();                  // imgui 컨텍스트 제거

	SDL_GL_DeleteContext(gl_context);         // SDL 컨텍스트 제거
	SDL_DestroyWindow(window);                // 윈도우 창 제거
	SDL_Quit();                               // SDL 종료

	return 0;
}
