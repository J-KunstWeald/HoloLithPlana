#include "hlp_d3d12.hh"
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>      
#include <d3dcompiler.h>  
#include <DirectXMath.h> 
#include "../hlp_math.hh"
#include "../app/hlp_init.hh"

namespace hlp::d12
{
	// STRUCTS
	// ==========================================================

	struct FConstBuffer
	{
		DirectX::XMMATRIX mvp;
	};

	struct FVertex
	{
		F32 x;
		F32 y;
		F32 z;
		F32 r;
		F32 g;
		F32 b;
		F32 a;
	};

	// GLOBALS
	// ==========================================================


	// Util
	// ------------------
	// Hres
	HRESULT g_hr;


	// Control
	// ------------------
	// Device
	ID3D12Device* g_pDev;
	// SwapChain
	IDXGISwapChain3* g_pSwpChn;
	// CommQueue
	ID3D12CommandQueue* g_pComQue;
	// CommList
	ID3D12GraphicsCommandList* g_pComList;
	// pertarget command allc
	ID3D12CommandAllocator* g_apComAlcs[HLP_DEFAULT_D12_NUMFRAMES];
	// Factory
	IDXGIFactory4* g_pFactory;

	// RootSig and PSO
	// ------------------
	// PSO
	ID3D12PipelineState* g_apPSO;
	// RS
	ID3D12RootSignature* g_pRootSig;

	// RTV
	// ------------------
	// RTV descheap
	ID3D12DescriptorHeap* g_pTargetDescHeap;
	I32 g_TargetDescHeapSize;
	// rendertargets
	ID3D12Resource* g_apTargets[HLP_DEFAULT_D12_NUMFRAMES];


	// Fences
	// ------------------
	// pertarget fence 
	ID3D12Fence* g_apFences[HLP_DEFAULT_D12_NUMFRAMES];
	// pertarget Fence val
	U64 g_aFenceVal[HLP_DEFAULT_D12_NUMFRAMES];
	// fenceEvent
	U0* g_pFenceEvent;
	// Current frame Index
	I32 g_CurrFrameIdx;


	// Viewing 
	// ------------------
	// Viewport
	D3D12_VIEWPORT g_ViewPort;
	// ScissorRect
	D3D12_RECT g_ScissorRect;


	// Depth Stencil Buffer 
	// ------------------
	ID3D12Resource* g_pDSBuf;
	ID3D12DescriptorHeap* g_pDSDescHeap;


	// Vertex Buffer 
	// ------------------
	ID3D12Resource* g_pVBuf;
	D3D12_VERTEX_BUFFER_VIEW g_VBufView;


	// Index Buffer 
	// ------------------
	ID3D12Resource* g_pIBuf;
	D3D12_INDEX_BUFFER_VIEW g_IBufView;


	// Const Buffer 
	// ------------------
	ID3D12Resource* g_apCBufUploadHeaps[HLP_DEFAULT_D12_NUMFRAMES];
	I32 g_CBufObjAlignedSize = (sizeof(FConstBuffer) + 255) & ~255;
	U8* g_apCBufViewGPUAddr[HLP_DEFAULT_D12_NUMFRAMES];
	FConstBuffer cbPerObject;


	DirectX::XMMATRIX g_VP;

	// LOCAL FUNCTIONS
	// ==========================================================

	// Init
	// ------------------
	U0 InitFactory(FAppState* pAS);
	U0 InitDevice(FAppState* pAS);
	U0 InitCommandQueue(FAppState* pAS);
	U0 InitSwapChain(FAppState* pAS);
	U0 InitRenderTargetViews(FAppState* pAS);
	U0 InitCommandAllocators(FAppState* pAS);
	U0 InitCommandList(FAppState* pAS);
	U0 InitFences(FAppState* pAS);
	U0 InitRootSignature(FAppState* pAS);
	U0 InitPSOs(FAppState* pAS);
	U0 InitVertexBuffer(FAppState* pAS);
	U0 InitIndexBuffer(FAppState* pAS);
	U0 InitDepthStencilBuffer(FAppState* pAS);
	U0 InitConstBuffer(FAppState* pAS);
	U0 InitFinalize(FAppState* pAS);

	// Runtime
	// ------------------

	// CleanUp
	// ------------------

	// LOCAL IMPL
	// ----------------------------------------------------------

	U0 InitFactory(FAppState* pAS)
	{
		U32 FactoryFlags = 0;
#if defined(_DEBUG)
		FactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		g_hr = CreateDXGIFactory2(FactoryFlags, __uuidof(IDXGIFactory4), reinterpret_cast<void**>(&g_pFactory));
		HLP_HANDLE_HRESULT(g_hr);

		ID3D12Debug* TempDebug = NULL;
		g_hr = D3D12GetDebugInterface(IID_PPV_ARGS(&TempDebug));
		HLP_HANDLE_HRESULT(g_hr);
		TempDebug->EnableDebugLayer();
		HLP_RELEASE(TempDebug);
	}

	U0 InitDevice(FAppState* pAS)
	{
		g_hr = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), reinterpret_cast<void**>(&g_pDev));
		HLP_HANDLE_HRESULT(g_hr);

		ID3D12InfoQueue* info_queue = NULL;
		g_pDev->QueryInterface(IID_PPV_ARGS(&info_queue));
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		HLP_RELEASE(info_queue);
	}

	U0 InitCommandQueue(FAppState* pAS)
	{
		D3D12_COMMAND_QUEUE_DESC ComQueDesc = {};
		ComQueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ComQueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

		g_hr = g_pDev->CreateCommandQueue(&ComQueDesc, IID_PPV_ARGS(&g_pComQue)); // create the command queue
		HLP_HANDLE_HRESULT(g_hr);
	}

	U0 InitSwapChain(FAppState* pAS)
	{
		DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
		backBufferDesc.Width = pAS->Width; // buffer width
		backBufferDesc.Height = pAS->Height; // buffer height
		backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

		// describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)
		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = pAS->pRenderState->NumFrames; // number of buffers we have
		swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
		swapChainDesc.OutputWindow = pAS->hWnd; // handle to our window
		swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
		swapChainDesc.Windowed = TRUE; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

		IDXGISwapChain* tempSwapChain;

		g_pFactory->CreateSwapChain(
			g_pComQue, // the queue will be flushed once the swap chain is created
			&swapChainDesc, // give it the swap chain description we created above
			&tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
		);

		g_pSwpChn = static_cast<IDXGISwapChain3*>(tempSwapChain);

		g_CurrFrameIdx = g_pSwpChn->GetCurrentBackBufferIndex();

	}

	U0 InitRenderTargetViews(FAppState* pAS)
	{
		D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
		RTVHeapDesc.NumDescriptors = pAS->pRenderState->NumFrames; // number of descriptors for this heap.
		RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap
		RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		g_hr = g_pDev->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&g_pTargetDescHeap));
		HLP_HANDLE_HRESULT(g_hr);

		// Devices may have different desc heap sizes, so we query it
		g_TargetDescHeapSize = g_pDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_CPU_DESCRIPTOR_HANDLE hTargetView = g_pTargetDescHeap->GetCPUDescriptorHandleForHeapStart();

		for (int i = 0; i < pAS->pRenderState->NumFrames; i++)
		{
			g_hr = g_pSwpChn->GetBuffer(i, IID_PPV_ARGS(&g_apTargets[i]));
			HLP_HANDLE_HRESULT(g_hr);

			g_pDev->CreateRenderTargetView(g_apTargets[i], NULL, hTargetView);

			hTargetView.ptr += g_TargetDescHeapSize;
		}
	}

	U0 InitCommandAllocators(FAppState* pAS)
	{
		for (int i = 0; i < pAS->pRenderState->NumFrames; i++)
		{
			g_hr = g_pDev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_apComAlcs[i]));
			HLP_HANDLE_HRESULT(g_hr);
		}
	}

	U0 InitCommandList(FAppState* pAS)
	{
		g_hr = g_pDev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_apComAlcs[g_CurrFrameIdx], NULL, IID_PPV_ARGS(&g_pComList));
		HLP_HANDLE_HRESULT(g_hr);
	}

	U0 InitFences(FAppState* pAS)
	{
		// create the fences
		for (int i = 0; i < pAS->pRenderState->NumFrames; i++)
		{
			g_hr = g_pDev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_apFences[i]));
			HLP_HANDLE_HRESULT(g_hr);
			g_aFenceVal[i] = 0; // set the initial fence value to 0
		}

		// create a handle to a fence event
		g_pFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		HLP_ASSERT(g_pFenceEvent);
	}

	U0 InitRootSignature(FAppState* pAS)
	{
#if 0
		// create a root descriptor, which explains where to find the data for this root parameter
		D3D12_ROOT_DESCRIPTOR RootCBVDescriptor = {};
		RootCBVDescriptor.RegisterSpace = 0;
		RootCBVDescriptor.ShaderRegister = 0;

		// create a root parameter and fill it out
		D3D12_ROOT_PARAMETER  RootParams[1] = {}; // only one parameter right now
		RootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
		RootParams[0].Descriptor = RootCBVDescriptor; // this is the root descriptor for this root parameter
		RootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now

		D3D12_ROOT_SIGNATURE_DESC RootSigDesc = {};
		RootSigDesc.NumParameters = _countof(RootParams);
		RootSigDesc.pParameters = RootParams;
		RootSigDesc.NumStaticSamplers = 0; // no static samplers
		RootSigDesc.pStaticSamplers = NULL;
		RootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; //|
			//D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;


		ID3DBlob* Signature = {};
		g_hr = D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, nullptr);
		HLP_HANDLE_HRESULT(g_hr);

		g_hr = g_pDev->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&g_pRootSig));
		HLP_HANDLE_HRESULT(g_hr);
#else
		// 1) Zero-init all structs
		D3D12_ROOT_DESCRIPTOR       cbvDesc = {};
		D3D12_ROOT_PARAMETER        rootParams[1] = {};
		D3D12_ROOT_SIGNATURE_DESC   sigDesc = {};

		// 2) Fill in CBV root parameter
		cbvDesc.RegisterSpace = 0;
		cbvDesc.ShaderRegister = 0;

		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParams[0].Descriptor = cbvDesc;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// 3) Fill out root-signature descriptor
		sigDesc.NumParameters = _countof(rootParams);
		sigDesc.pParameters = rootParams;
		sigDesc.NumStaticSamplers = 0;
		sigDesc.pStaticSamplers = nullptr;
		sigDesc.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;// |
		/*	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;*/
		// (no deny-pixel, since you have a pixel shader)

	// 4) Serialize
		ID3DBlob* signatureBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(
			&sigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&signatureBlob,
			&errorBlob
		);
		if (FAILED(hr))
		{
			if (errorBlob)
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			  // Or your equivalent failure code
		}

		// 5) Create the root signature
		hr = g_pDev->CreateRootSignature(
			0,
			signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&g_pRootSig)
		);
		signatureBlob->Release();
		if (errorBlob) errorBlob->Release();
		HLP_HANDLE_HRESULT(hr);

#endif
	}

	U0 InitPSOs(FAppState* pAS)
	{
		// create vertex and pixel shaders

// when debugging, we can compile the shader files at runtime.
// but for release versions, we can compile the hlsl shaders
// with fxc.exe to create .cso files, which contain the shader
// bytecode. We can load the .cso files at runtime to get the
// shader bytecode, which of course is faster than compiling
// them at runtime

// compile vertex shader
		ID3DBlob* vertexShader; // d3d blob for holding vertex shader bytecode
		ID3DBlob* errorBuff; // a buffer holding the error data if any
		g_hr = D3DCompileFromFile(HLP_D3D12_VS_LOC,
			nullptr,
			nullptr,
			"main",
			"vs_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&vertexShader,
			&errorBuff);
		HLP_HANDLE_HRESULT(g_hr);

		// fill out a shader bytecode structure, which is basically just a pointer
		// to the shader bytecode and the size of the shader bytecode
		D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
		vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
		vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

		// compile pixel shader
		ID3DBlob* pixelShader;
		g_hr = D3DCompileFromFile(HLP_D3D12_PS_LOC,
			nullptr,
			nullptr,
			"main",
			"ps_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&pixelShader,
			&errorBuff);
		HLP_HANDLE_HRESULT(g_hr);

		// fill out shader bytecode structure for pixel shader
		D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
		pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
		pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

		// create input layout

		// The input layout is used by the Input Assembler so that it knows
		// how to read the vertex data bound to it.

		D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// fill out an input layout description structure
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

		// we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
		inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
		inputLayoutDesc.pInputElementDescs = inputLayout;

		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)


		// Depth Stencil Desc
		D3D12_DEPTH_STENCIL_DESC DSD = {};

		DSD.DepthEnable = TRUE;
		DSD.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		DSD.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		DSD.StencilEnable = FALSE;
		DSD.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		DSD.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
		{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		DSD.FrontFace = defaultStencilOp;
		DSD.BackFace = defaultStencilOp;

		// Blend Desc
		D3D12_BLEND_DESC BD = {};
		BD.AlphaToCoverageEnable = FALSE;
		BD.IndependentBlendEnable = FALSE;
		D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
		{
			FALSE,FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		for (U32 i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		{
			BD.RenderTarget[i] = defaultRenderTargetBlendDesc;
		}

		// Rasterizer Desc SOLID
		D3D12_RASTERIZER_DESC RD = {};
		RD.FillMode = D3D12_FILL_MODE_SOLID;
		RD.CullMode = D3D12_CULL_MODE_BACK;
		RD.FrontCounterClockwise = FALSE;
		RD.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		RD.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RD.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RD.DepthClipEnable = TRUE;
		RD.MultisampleEnable = FALSE;
		RD.AntialiasedLineEnable = FALSE;
		RD.ForcedSampleCount = 0;
		RD.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
		psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
		psoDesc.pRootSignature = g_pRootSig; // the root signature that describes the input data this pso needs
		assert(psoDesc.pRootSignature == g_pRootSig);
		psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
		psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
		psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
		psoDesc.RasterizerState = RD; // a default rasterizer state.
		psoDesc.BlendState = BD; // a default blent state.
		psoDesc.NumRenderTargets = 1; // we are only binding one render target
		psoDesc.DepthStencilState = DSD; // a default depth stencil state

		// 0 is solid
		g_hr = g_pDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_apPSO));
		HLP_HANDLE_HRESULT(g_hr);

		//RD.FillMode = D3D12_FILL_MODE_WIREFRAME;
		//psoDesc.RasterizerState = RD;

		//// 1 is WF
		//g_hr = g_pDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_apPSO[1]));
		//HLP_HANDLE_HRESULT(g_hr);
	}

	U0 InitVertexBuffer(FAppState* pAS)
	{
		// load meshes
		FVertex* pVerts = (FVertex*)malloc(sizeof(FVertex) * pAS->pRenderState->TotNumVerts);
		U32 CurrentIndex = 0;
		for (int i = 0; i < pAS->pSceneState->NumGeometries; i++)
		{
			if (pAS->pSceneState->Geometries[i].Type == MESH)
			{
				for (int j = 0; j < pAS->pSceneState->Geometries[i].pData.pMesh->NumVerts; j++)
				{
					pVerts[CurrentIndex] = FVertex{
					pAS->pSceneState->Geometries[i].pData.pMesh->Verts[j].x,
					pAS->pSceneState->Geometries[i].pData.pMesh->Verts[j].y,
					pAS->pSceneState->Geometries[i].pData.pMesh->Verts[j].z,
					0.8f,
					0.9f,
					0.2f,
					0.0f
					};

					CurrentIndex++;
				}
			}
		}
		HLP_ASSERT(CurrentIndex == pAS->pRenderState->TotNumVerts);

		I32 VertBufSize = pAS->pRenderState->TotNumVerts;

		// Heap properties
		D3D12_HEAP_PROPERTIES DefaultHeapProps;
		DefaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		DefaultHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		DefaultHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		DefaultHeapProps.CreationNodeMask = 1;
		DefaultHeapProps.VisibleNodeMask = 1;

		// Resource Desc
		D3D12_RESOURCE_DESC ResDesc;
		ResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ResDesc.Alignment = 0;
		ResDesc.Width = VertBufSize;
		ResDesc.Height = 1;
		ResDesc.DepthOrArraySize = 1;
		ResDesc.MipLevels = 1;
		ResDesc.Format = DXGI_FORMAT_UNKNOWN;
		ResDesc.SampleDesc.Count = 1;
		ResDesc.SampleDesc.Quality = 0;
		ResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		ResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// create default heap
		// default heap is memory on the GPU. Only the GPU has access to this memory
		// To get data into this heap, we will have to upload the data using
		// an upload heap
		g_hr = g_pDev->CreateCommittedResource(
			&DefaultHeapProps, // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&ResDesc, // resource description for a buffer
			D3D12_RESOURCE_STATE_COMMON, // we will start this heap in the copy destination state since we will copy data
			// from the upload heap to this heap
			nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&g_pVBuf));
		HLP_HANDLE_HRESULT(g_hr);

		g_pVBuf->SetName(L"V Buffer Resource Heap");

		// Upload heap props
		D3D12_HEAP_PROPERTIES UploadHeapProps;
		UploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		UploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		UploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		UploadHeapProps.CreationNodeMask = 1;
		UploadHeapProps.VisibleNodeMask = 1;

		// create upload heap
		// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
		// We will upload the vertex buffer using this heap to the default heap
		ID3D12Resource* vBufferUploadHeap;
		g_hr = g_pDev->CreateCommittedResource(
			&UploadHeapProps, // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&ResDesc, // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(&vBufferUploadHeap));
		vBufferUploadHeap->SetName(L"V Buffer Upload Resource Heap");
		HLP_HANDLE_HRESULT(g_hr);

		// store vertex buffer in upload heap
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(pVerts); // pointer to our vertex array
		vertexData.RowPitch = VertBufSize; // size of all our triangle vertex data
		vertexData.SlicePitch = VertBufSize; // also the size of our triangle vertex data

		// we are now creating a command with the command list to copy the data from
		// the upload heap to the default heap
		BYTE* mapped = nullptr;
		D3D12_RANGE mapRange{ 0, 0 };  // We do not intend to read back from this memory.
		vBufferUploadHeap->Map(0, &mapRange, reinterpret_cast<void**>(&mapped));
		memcpy(mapped, pVerts, VertBufSize);
		vBufferUploadHeap->Unmap(0, nullptr);

		// 3) Record a copy from the upload heap into the default heap:
		g_pComList->CopyBufferRegion(
			g_pVBuf,      // dst buffer
			0,                  // dst offset
			vBufferUploadHeap,       // src buffer
			0,                  // src offset
			VertBufSize            // number of bytes to copy
		);

		// 4) Transition the default buffer into the state you need:
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = g_pVBuf;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		g_pComList->ResourceBarrier(1, &barrier);
		free(pVerts);
	}

	U0 InitIndexBuffer(FAppState* pAS)
	{
		U32* pIndices = (U32*)malloc(sizeof(U32) * pAS->pRenderState->TotNumIdx);
		U32 CurrentIndex = 0;
		for (int i = 0; i < pAS->pSceneState->NumGeometries; i++)
		{
			if (pAS->pSceneState->Geometries[i].Type == MESH)
			{
				for (int j = 0; j < pAS->pSceneState->Geometries[i].pData.pMesh->NumIdx; j++)
				{
					pIndices[CurrentIndex] = pAS->pSceneState->Geometries[i].pData.pMesh->Idx[j];
					CurrentIndex++;
				}
				//PrintFMesh(pAppState->pSceneState->Geometries[i].pData.pMesh);
			}
		}
		HLP_ASSERT(CurrentIndex == pAS->pRenderState->TotNumIdx);

		I32 IbufSize = pAS->pRenderState->TotNumIdx;

		// Heap properties
		D3D12_HEAP_PROPERTIES DefaultHeapProps;
		DefaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		DefaultHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		DefaultHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		DefaultHeapProps.CreationNodeMask = 1;
		DefaultHeapProps.VisibleNodeMask = 1;

		// Resource Desc
		D3D12_RESOURCE_DESC ResDesc;
		ResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ResDesc.Alignment = 0;
		ResDesc.Width = IbufSize;
		ResDesc.Height = 1;
		ResDesc.DepthOrArraySize = 1;
		ResDesc.MipLevels = 1;
		ResDesc.Format = DXGI_FORMAT_UNKNOWN;
		ResDesc.SampleDesc.Count = 1;
		ResDesc.SampleDesc.Quality = 0;
		ResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		ResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// create default heap
		// default heap is memory on the GPU. Only the GPU has access to this memory
		// To get data into this heap, we will have to upload the data using
		// an upload heap
		g_hr = g_pDev->CreateCommittedResource(
			&DefaultHeapProps, // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&ResDesc, // resource description for a buffer
			D3D12_RESOURCE_STATE_COMMON, // we will start this heap in the copy destination state since we will copy data
			// from the upload heap to this heap
			nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&g_pIBuf));
		HLP_HANDLE_HRESULT(g_hr);

		g_pIBuf->SetName(L"V Buffer Resource Heap");

		// Upload heap props
		D3D12_HEAP_PROPERTIES UploadHeapProps;
		UploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		UploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		UploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		UploadHeapProps.CreationNodeMask = 1;
		UploadHeapProps.VisibleNodeMask = 1;

		// create upload heap
		// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
		// We will upload the vertex buffer using this heap to the default heap
		ID3D12Resource* IBufferUploadHeap;
		g_hr = g_pDev->CreateCommittedResource(
			&UploadHeapProps, // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&ResDesc, // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(&IBufferUploadHeap));
		IBufferUploadHeap->SetName(L"V Buffer Upload Resource Heap");
		HLP_HANDLE_HRESULT(g_hr);

		// store vertex buffer in upload heap
		D3D12_SUBRESOURCE_DATA IndexData = {};
		IndexData.pData = reinterpret_cast<BYTE*>(pIndices); // pointer to our vertex array
		IndexData.RowPitch = IbufSize; // size of all our triangle vertex data
		IndexData.SlicePitch = IbufSize; // also the size of our triangle vertex data

		// we are now creating a command with the command list to copy the data from
		// the upload heap to the default heap
		BYTE* mapped = nullptr;
		D3D12_RANGE mapRange{ 0, 0 };  // We do not intend to read back from this memory.
		IBufferUploadHeap->Map(0, &mapRange, reinterpret_cast<void**>(&mapped));
		memcpy(mapped, pIndices, IbufSize);
		IBufferUploadHeap->Unmap(0, nullptr);

		// 3) Record a copy from the upload heap into the default heap:
		g_pComList->CopyBufferRegion(
			g_pIBuf,      // dst buffer
			0,                  // dst offset
			IBufferUploadHeap,       // src buffer
			0,                  // src offset
			IbufSize            // number of bytes to copy
		);

		// 4) Transition the default buffer into the state you need:
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = g_pIBuf;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		g_pComList->ResourceBarrier(1, &barrier);


		free(pIndices);
	}

	U0 InitDepthStencilBuffer(FAppState* pAS)
	{
		// Upload heap props
		D3D12_HEAP_PROPERTIES UploadHeapProps;
		UploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		UploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		UploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		UploadHeapProps.CreationNodeMask = 1;
		UploadHeapProps.VisibleNodeMask = 1;

		// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		g_hr = g_pDev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&g_pDSDescHeap));
		HLP_HANDLE_HRESULT(g_hr);

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;


		D3D12_RESOURCE_DESC DSResDesc;
		DSResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		DSResDesc.Alignment = 0;
		DSResDesc.Width = pAS->Width;
		DSResDesc.Height = pAS->Height;
		DSResDesc.DepthOrArraySize = 1;
		DSResDesc.MipLevels = 0;
		DSResDesc.Format = DXGI_FORMAT_D32_FLOAT;
		DSResDesc.SampleDesc.Count = 1;
		DSResDesc.SampleDesc.Quality = 0;
		DSResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		DSResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		g_pDev->CreateCommittedResource(
			&UploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&DSResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&g_pDSBuf)
		);
		g_pDSDescHeap->SetName(L"Depth/Stencil Resource Heap");

		g_pDev->CreateDepthStencilView(g_pDSBuf, &depthStencilDesc, g_pDSDescHeap->GetCPUDescriptorHandleForHeapStart());

	}

	U0 InitConstBuffer(FAppState* pAS)
	{
		// Resource Desc
		D3D12_RESOURCE_DESC ResDesc;
		ResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ResDesc.Alignment = 0;
		ResDesc.Width = 1024 * 64; // 64 kB
		ResDesc.Height = 1;
		ResDesc.DepthOrArraySize = 1;
		ResDesc.MipLevels = 1;
		ResDesc.Format = DXGI_FORMAT_UNKNOWN;
		ResDesc.SampleDesc.Count = 1;
		ResDesc.SampleDesc.Quality = 0;
		ResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		ResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// Upload heap props
		D3D12_HEAP_PROPERTIES UploadHeapProps;
		UploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		UploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		UploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		UploadHeapProps.CreationNodeMask = 1;
		UploadHeapProps.VisibleNodeMask = 1;

		for (int i = 0; i < pAS->pRenderState->NumFrames; ++i)
		{
			// create resource for cube 1
			g_hr = g_pDev->CreateCommittedResource(
				&UploadHeapProps, // this heap will be used to upload the constant buffer data
				D3D12_HEAP_FLAG_NONE, // no flags
				&ResDesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
				D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
				nullptr, // we do not have use an optimized clear value for constant buffers
				IID_PPV_ARGS(&g_apCBufUploadHeaps[i]));
			g_apCBufUploadHeaps[i]->SetName(L"Constant Buffer Upload Resource Heap");

			ZeroMemory(&cbPerObject, sizeof(FConstBuffer));

			D3D12_RANGE readRange = { 0,0 };	// We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

			// map the resource heap to get a gpu virtual address to the beginning of the heap
			g_hr = g_apCBufUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&g_apCBufViewGPUAddr[i]));

			for (int j = 0; j < pAS->pSceneState->NumGeometries; j++)
			{
				memcpy(g_apCBufViewGPUAddr[i] + (g_CBufObjAlignedSize * j), &cbPerObject, sizeof(cbPerObject));
			}
			// Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
			// so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.
		 // cube1's constant buffer data
			 // cube2's constant buffer data
		}
	}

	U0 InitFinalize(FAppState* pAS)
	{
		// Now we execute the command list to upload the initial assets (triangle data)
		g_pComList->Close();
		ID3D12CommandList* ppCommandLists[] = { g_pComList };
		g_pComQue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
		g_aFenceVal[g_CurrFrameIdx]++;
		g_hr = g_pComQue->Signal(g_apFences[g_CurrFrameIdx], g_aFenceVal[g_CurrFrameIdx]);


		// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
		g_VBufView.BufferLocation = g_pVBuf->GetGPUVirtualAddress();
		g_VBufView.StrideInBytes = sizeof(FVertex);
		g_VBufView.SizeInBytes = pAS->pRenderState->TotNumVerts * sizeof(FVertex);

		// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
		g_IBufView.BufferLocation = g_pIBuf->GetGPUVirtualAddress();
		g_IBufView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
		g_IBufView.SizeInBytes = pAS->pRenderState->TotNumIdx * sizeof(I32);

		// Fill out the Viewport
		g_ViewPort.TopLeftX = 0;
		g_ViewPort.TopLeftY = 0;
		g_ViewPort.Width = pAS->Width;
		g_ViewPort.Height = pAS->Height;
		g_ViewPort.MinDepth = 0.0f;
		g_ViewPort.MaxDepth = 1.0f;

		// Fill out a scissor rect
		g_ScissorRect.left = 0;
		g_ScissorRect.top = 0;
		g_ScissorRect.right = pAS->Width;
		g_ScissorRect.bottom = pAS->Height;
	}

	// Start Runtime

	U0 WaitForPreviousFrame()
	{
		HRESULT hr;

		// swap the current rtv buffer index so we draw on the correct buffer
		g_CurrFrameIdx = g_pSwpChn->GetCurrentBackBufferIndex();

		// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
		// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
		if (g_apFences[g_CurrFrameIdx]->GetCompletedValue() < g_aFenceVal[g_CurrFrameIdx])
		{
			// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
			hr = g_apFences[g_CurrFrameIdx]->SetEventOnCompletion(g_aFenceVal[g_CurrFrameIdx], g_pFenceEvent);


			// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
			// has reached "fenceValue", we know the command queue has finished executing
			WaitForSingleObject(g_pFenceEvent, INFINITE);
		}

		// increment fenceValue for next frame
		g_aFenceVal[g_CurrFrameIdx]++;
	}

	U0 UpdatePipeline(FAppState* pAS)
	{
		printf("SetGraphicsRootSignature: %p\n", g_pRootSig);
		// View proj matrix
		g_VP = DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(pAS->pSceneState->Cam.CameraPosition.x, pAS->pSceneState->Cam.CameraPosition.y, pAS->pSceneState->Cam.CameraPosition.z, 0.0f),
			DirectX::XMVectorSet(pAS->pSceneState->Cam.CameraForward.x, pAS->pSceneState->Cam.CameraForward.y, pAS->pSceneState->Cam.CameraForward.z, 0.0f),
			DirectX::XMVectorSet(pAS->pSceneState->Cam.CameraUp.x, pAS->pSceneState->Cam.CameraUp.y, pAS->pSceneState->Cam.CameraUp.z, 0.0f)
		)
			*
			DirectX::XMMatrixPerspectiveFovLH(
				HLP_TO_RADIAN(pAS->pSceneState->Cam.HorizontalFOVDeg),
				pAS->pSceneState->Cam.AspectRatio,
				pAS->pSceneState->Cam.NearPlane,
				pAS->pSceneState->Cam.FarPlane
			);

		// Copy em
		for (int j = 0; j < pAS->pSceneState->NumGeometries; j++)
		{
			if (j % 2 == 0)
			{
				cbPerObject.mvp = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(0.15f, 0.1f, 2.0f) * g_VP);
			}
			else
			{
				cbPerObject.mvp = DirectX::XMMatrixTranspose(g_VP);
			}
			memcpy(g_apCBufViewGPUAddr[j] + (g_CBufObjAlignedSize * j), &cbPerObject, sizeof(cbPerObject));
		}
		printf("SetGraphicsRootSignature: %p\n", g_pRootSig);
		WaitForPreviousFrame();
		printf("SetGraphicsRootSignature: %p\n", g_pRootSig);
		//g_hr = g_apComAlcs[g_CurrFrameIdx]->Reset();
		//HLP_HANDLE_HRESULT(g_hr);

		//g_hr = g_pComList->Reset(g_apComAlcs[g_CurrFrameIdx], g_apPSO);
		//HLP_HANDLE_HRESULT(g_hr);
		assert(SUCCEEDED(g_apComAlcs[g_CurrFrameIdx]->Reset()));
		assert(SUCCEEDED(g_pComList->Reset(g_apComAlcs[g_CurrFrameIdx], g_apPSO)));


		D3D12_RESOURCE_BARRIER FirstBarrier = {};
		FirstBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		FirstBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		FirstBarrier.Transition.pResource = g_apTargets[g_CurrFrameIdx];
		FirstBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		FirstBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		FirstBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		printf("SetGraphicsRootSignature: %p\n", g_pRootSig);
		g_pComList->ResourceBarrier(1, &FirstBarrier);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pTargetDescHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += SIZE_T(g_CurrFrameIdx) * SIZE_T(g_TargetDescHeapSize);

		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = g_pDSDescHeap->GetCPUDescriptorHandleForHeapStart();
		// Now prepare draws

		// set the render target for the output merger stage (the output of the pipeline)
		g_pComList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		// Clear the render target by using the ClearRenderTargetView command
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		g_pComList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		assert(g_pComList != nullptr);

		// clear the depth/stencil buffer
		g_pComList->ClearDepthStencilView(g_pDSDescHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// set root signature
		printf("SetGraphicsRootSignature: %p\n", g_pRootSig);
		g_pComList->SetGraphicsRootSignature(g_pRootSig); // set the root signature

		// draw triangle
		g_pComList->RSSetViewports(1, &g_ViewPort); // set the viewports
		g_pComList->RSSetScissorRects(1, &g_ScissorRect); // set the scissor rects
		g_pComList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
		g_pComList->IASetVertexBuffers(0, 1, &g_VBufView); // set the vertex buffer (using the vertex buffer view)
		g_pComList->IASetIndexBuffer(&g_IBufView);


		U32 CurrentVBufIdx = 0;
		U32 CurrentIBufIdx = 0;
		for (int i = 0; i < pAS->pSceneState->NumGeometries; i++)
		{
			g_pComList->SetGraphicsRootConstantBufferView(0, g_apCBufUploadHeaps[g_CurrFrameIdx]->GetGPUVirtualAddress() + (g_CBufObjAlignedSize * i));
			g_pComList->DrawIndexedInstanced(pAS->pSceneState->Geometries[i].pData.pMesh->NumIdx, 1,CurrentIBufIdx, CurrentVBufIdx,0);
			CurrentIBufIdx += pAS->pSceneState->Geometries[i].pData.pMesh->NumIdx;
			CurrentVBufIdx += pAS->pSceneState->Geometries[i].pData.pMesh->NumVerts;
		}

		// Barrier and close
		D3D12_RESOURCE_BARRIER SecondBarrier = {};
		SecondBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		SecondBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		SecondBarrier.Transition.pResource = g_apTargets[g_CurrFrameIdx];
		SecondBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		SecondBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		SecondBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		g_pComList->ResourceBarrier(1, &SecondBarrier);

		g_hr = g_pComList->Close();
		HLP_HANDLE_HRESULT(g_hr);
	}

	void Render(FAppState* pAS)
	{
		HRESULT hr;

		UpdatePipeline(pAS); // update the pipeline by sending commands to the commandqueue

		// create an array of command lists (only one command list here)
		ID3D12CommandList* ppCommandLists[] = { g_pComList };

		// execute the array of command lists
		g_pComQue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// this command goes in at the end of our command queue. we will know when our command queue 
		// has finished because the fence value will be set to "fenceValue" from the GPU since the command
		// queue is being executed on the GPU
		hr = g_pComQue->Signal(g_apFences[g_CurrFrameIdx], g_aFenceVal[g_CurrFrameIdx]);

		// present the current backbuffer
		hr = g_pSwpChn->Present(0, 0);

	}

	// EXPORT FUNCTIONS
	// ==========================================================

	U0 InitD3D12(FAppState* pAS)
	{
		InitFactory(pAS);
		InitDevice(pAS);
		InitCommandQueue(pAS);
		InitSwapChain(pAS);
		InitRenderTargetViews(pAS);
		InitCommandAllocators(pAS);
		InitCommandList(pAS);
		InitFences(pAS);
		InitRootSignature(pAS);
		printf("RootSig: %p\n", g_pRootSig);
		InitPSOs(pAS);
		InitVertexBuffer(pAS);
		InitIndexBuffer(pAS);
		InitDepthStencilBuffer(pAS);
		InitConstBuffer(pAS);
		InitFinalize(pAS);
		assert(g_pRootSig != nullptr);
	}
	U0 DrawD3D12(FAppState* pAS)
	{
		Render(pAS);
	}
	U0 CleanUpD3D12(FAppState* pAS)
	{
		g_pRootSig->Release();
	}

}