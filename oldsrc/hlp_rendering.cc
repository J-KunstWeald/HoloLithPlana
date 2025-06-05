
#define UNICODE 1
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <d3dcompiler.h>
#include <Directxmath.h>

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <cmath>
#include <malloc.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "hlp_defines.hh"
#include "hlp_rendering.hh"
#include "hlp_input.hh"
#include "hlp_geometry.hh"
#include "hlp_gamelogic.hh"
#include "stdio.h"

namespace hlp
{
#define HLP_HANDLE_HRESULT(hr) \
    do { \
        if (FAILED(hr)) { \
            printf("FAILED: %s, %d\n", __FILE__, __LINE__); \
        } \
    } while (0)

	// +---------+
	// | Structs |
	// +---------+

	struct VERTEX
	{
		FLOAT X;
		FLOAT Y;
		FLOAT Z;
		D3DXCOLOR Color;
	};

	struct ConstBuffer
	{
		DirectX::XMFLOAT4X4 mvp;
	};

	// +---------+
	// | Forward |
	// +---------+

	U0 InitD3DBase(HWND* phWnd, FRenderState* pRenderState);
	U0 InitDepthBuf(FRenderState* pRenderState);
	U0 InitUtils(HWND hWnd, FRenderState* pRenderState);

	U0 InitIndexBuffer(FResource* pResources, FRenderState* pRenderState);
	U0 InitVertBuf(FResource* pResources, FRenderState* pRenderState);
	U0 InitConstBuf();

	DirectX::XMFLOAT4X4 XMMathComputeMVP(FCamera* Cam);

	// +---------+
	// | Globals |
	// +---------+
	
	// Core
	IDXGISwapChain* g_pSwapChain;             
	ID3D11Device* g_pDevice;                     
	ID3D11DeviceContext* g_pDevCon;           
	// Shader
	ID3D11VertexShader* g_pVS;				
	ID3D11PixelShader* g_pPS;					
	// Buffer
	ID3D11Buffer* g_pVBuffer;					
	ID3D11Buffer* g_pCBuffer;	
	ID3D11Buffer* g_pIBuffer;
	// Misc
	ID3D11InputLayout* g_pLayout;
	ID3D11RenderTargetView* g_pBackbuffer;
	ID3D11DepthStencilView* g_pDepthStencilView;
	ID3D11Texture2D* g_pDepthStencilBuffer;

	HRESULT hr;

	// Timer
	U0 InitUtils(HWND hWnd, FRenderState* pRenderState)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(g_pDevice, g_pDevCon);

		// Init timer
		LARGE_INTEGER TimerFrequency;
		QueryPerformanceFrequency(&TimerFrequency);
		pRenderState->TimerFrequency = (U64)TimerFrequency.QuadPart;
	}


	DirectX::XMFLOAT4X4 XMMathComputeMVP(FCamera* Cam)
	{
		DirectX::XMMATRIX XMmodel = DirectX::XMMatrixIdentity();

		DirectX::XMVECTOR XMCamPos = DirectX::XMVectorSet(Cam->CameraPosition.x, Cam->CameraPosition.y, Cam->CameraPosition.z, 0.0f);
		DirectX::XMVECTOR XMCameraForward = DirectX::XMVectorSet(Cam->CameraForward.x, Cam->CameraForward.y, Cam->CameraForward.z, 0.0f); // where camera looks
		DirectX::XMVECTOR XMUp = DirectX::XMVectorSet(Cam->CameraUp.x, Cam->CameraUp.y, Cam->CameraUp.z, 0.0f); // up vector

		DirectX::XMMATRIX XMview = DirectX::XMMatrixLookAtLH(XMCamPos, XMCameraForward, XMUp); // DirectX::XMVectorAdd(XMCamPos, XMCameraForward)

		float fovAngleY = DirectX::XMConvertToRadians(Cam->HorizontalFOVDeg);

		DirectX::XMMATRIX XMprojection = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, Cam->AspectRatio, Cam->NearPlane, Cam->FarPlane);

		DirectX::XMMATRIX mvp = XMmodel * XMview * XMprojection;

		DirectX::XMFLOAT4X4 res_mvp;
		DirectX::XMFLOAT4X4 matM;
		DirectX::XMFLOAT4X4 matV;
		DirectX::XMFLOAT4X4 matP;


		DirectX::XMStoreFloat4x4(&res_mvp, DirectX::XMMatrixTranspose(mvp));

		DirectX::XMStoreFloat4x4(&matM, XMmodel);
		DirectX::XMStoreFloat4x4(&matV, XMview);
		DirectX::XMStoreFloat4x4(&matP, XMprojection);

#if 0
		printf("MVP Matrix:\n");
		printf("[ %.3f %.3f %.3f %.3f ]\n", mat._11, mat._12, mat._13, mat._14);
		printf("[ %.3f %.3f %.3f %.3f ]\n", mat._21, mat._22, mat._23, mat._24);
		printf("[ %.3f %.3f %.3f %.3f ]\n", mat._31, mat._32, mat._33, mat._34);
		printf("[ %.3f %.3f %.3f %.3f ]\n", mat._41, mat._42, mat._43, mat._44);

		printf("M Matrix:\n");
		printf("[ %.3f %.3f %.3f %.3f ]\n", matM._11, matM._12, matM._13, matM._14);
		printf("[ %.3f %.3f %.3f %.3f ]\n", matM._21, matM._22, matM._23, matM._24);
		printf("[ %.3f %.3f %.3f %.3f ]\n", matM._31, matM._32, matM._33, matM._34);
		printf("[ %.3f %.3f %.3f %.3f ]\n", matM._41, matM._42, matM._43, matM._44);

		printf("V Matrix:\n");
		printf("[ %.3f %.3f %.3f %.3f ]\n", matV._11, matV._12, matV._13, matV._14);
		printf("[ %.3f %.3f %.3f %.3f ]\n", matV._21, matV._22, matV._23, matV._24);
		printf("[ %.3f %.3f %.3f %.3f ]\n", matV._31, matV._32, matV._33, matV._34);
		printf("[ %.3f %.3f %.3f %.3f ]\n", matV._41, matV._42, matV._43, matV._44);

		printf("P Matrix:\n");
		printf("[ %.3f %.3f %.3f %.3f ]\n", matP._11, matP._12, matP._13, matP._14);
		printf("[ %.3f %.3f %.3f %.3f ]\n", matP._21, matP._22, matP._23, matP._24);
		printf("[ %.3f %.3f %.3f %.3f ]\n", matP._31, matP._32, matP._33, matP._34);
		printf("[ %.3f %.3f %.3f %.3f ]\n", matP._41, matP._42, matP._43, matP._44);
#endif
		return res_mvp;
	}

	U0 InitD3DBase(HWND* phWnd, FRenderState* pRenderState)
	{
		
			// create a struct to hold information about the swap chain
			DXGI_SWAP_CHAIN_DESC SwapChainDescription;

			// clear out the struct for use
			ZeroMemory(&SwapChainDescription, sizeof(DXGI_SWAP_CHAIN_DESC));

			// fill the swap chain description struct
			SwapChainDescription.BufferCount = 1;                                    // one back buffer
			
			SwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
			SwapChainDescription.BufferDesc.Width = pRenderState->Width;                    // set the back buffer width
			SwapChainDescription.BufferDesc.Height = pRenderState->Height;                  // set the back buffer height
			SwapChainDescription.BufferDesc.RefreshRate.Numerator = pRenderState->MaxFPS;
			SwapChainDescription.BufferDesc.RefreshRate.Denominator = 1; // NO IDEA
			SwapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // NO IDEA
			SwapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // NO IDEA
			 

			SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
			SwapChainDescription.OutputWindow = *phWnd;                                // the window to be used
			SwapChainDescription.SampleDesc.Count = 4;                               // how many multisamples
			SwapChainDescription.Windowed = TRUE;                                    // windowed/full-screen mode
			SwapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching

			// create a device, device context and swap chain using the information in the scd struct
			HLP_HANDLE_HRESULT(D3D11CreateDeviceAndSwapChain(NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				NULL,
				NULL,
				NULL,
				D3D11_SDK_VERSION,
				&SwapChainDescription,
				&g_pSwapChain,
				&g_pDevice,
				NULL,
				&g_pDevCon));


			// get the address of the back buffer
			ID3D11Texture2D* pBackBuffer;
			HLP_HANDLE_HRESULT(g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));

			// use the back buffer address to create the render target
			HLP_HANDLE_HRESULT(g_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pBackbuffer));
			pBackBuffer->Release();

			InitDepthBuf(pRenderState);

			// set the render target as the back buffer
			g_pDevCon->OMSetRenderTargets(1, &g_pBackbuffer, g_pDepthStencilView);

			// Set the viewport
			D3D11_VIEWPORT ViewPort;
			ZeroMemory(&ViewPort, sizeof(D3D11_VIEWPORT));

			ViewPort.TopLeftX = 0;
			ViewPort.TopLeftY = 0;
			ViewPort.Width = pRenderState->Width;
			ViewPort.Height = pRenderState->Height;
			// Dbuf
			ViewPort.MinDepth = 0.f;
			ViewPort.MaxDepth = 1.f;

			g_pDevCon->RSSetViewports(1, &ViewPort);
	}

	U0 InitConstBuf()
	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(ConstBuffer);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Create the buffer.
		HLP_HANDLE_HRESULT(g_pDevice->CreateBuffer(&cbDesc, nullptr,
			&g_pCBuffer)); // mid &InitData

		// Set the buffer.
		g_pDevCon->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	}

	U0 InitDepthBuf(FRenderState* pRenderState)
	{

		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width = pRenderState->Width;
		depthStencilDesc.Height = pRenderState->Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		HLP_HANDLE_HRESULT(g_pDevice->CreateTexture2D(&depthStencilDesc, NULL, &g_pDepthStencilBuffer));
		HLP_HANDLE_HRESULT(g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer, NULL, &g_pDepthStencilView));
	}

	U0 InitShaders(U0)
	{
		// load and compile the two shaders
		ID3D10Blob* VS;
		ID3D10Blob* PS;

		// broken
		//D3DX11CompileFromFile(L"../shaders.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);
		//D3DX11CompileFromFile(L"../shaders.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);

		HLP_HANDLE_HRESULT(D3DCompileFromFile(HLP_SHADER_LOCATION, 0, 0, "VShader", "vs_4_0", 0, 0, &VS, 0));
		HLP_HANDLE_HRESULT(D3DCompileFromFile(HLP_SHADER_LOCATION, 0, 0, "PShader", "ps_4_0", 0, 0, &PS, 0));

		// encapsulate both shaders into shader objects
		HLP_HANDLE_HRESULT(g_pDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &g_pVS));
		HLP_HANDLE_HRESULT(g_pDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &g_pPS));
		// set the shader objects
		g_pDevCon->VSSetShader(g_pVS, 0, 0);
		g_pDevCon->PSSetShader(g_pPS, 0, 0);

		// create the input layout object
		D3D11_INPUT_ELEMENT_DESC VertBufInpElemDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		HLP_HANDLE_HRESULT(g_pDevice->CreateInputLayout(VertBufInpElemDesc, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &g_pLayout));
		g_pDevCon->IASetInputLayout(g_pLayout);
	}

	U0 InitD3D(HWND* phWnd, FRenderState* pRenderState, FResource* pResources)
	{
		InitD3DBase(phWnd, pRenderState);
		InitShaders();
		InitVertBuf(pResources, pRenderState);
		InitConstBuf();
		InitIndexBuffer(pResources, pRenderState);
		InitUtils(*phWnd, pRenderState);
	}

	U0 InitVertBuf(FResource* pResources, FRenderState* pRenderState)
	{
		I32 CountOfVerts = 0;

		for (int i = 0; i < pRenderState->NumResources ; i++)
		{
			if (pResources[i].Type == MESH)
			{
				CountOfVerts += pResources->pData.pMesh->NumVerts;
			}
			else if (pResources[i].Type == POINTCLOUD)
			{
				CountOfVerts += pResources->pData.pPointcloud->NumVerts;
			}
		}

		pRenderState->NumVBuf = CountOfVerts;

		// create the vertex buffer
		D3D11_BUFFER_DESC VertBufDesc;
		ZeroMemory(&VertBufDesc, sizeof(VertBufDesc));

		VertBufDesc.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
		VertBufDesc.ByteWidth = sizeof(VERTEX) * CountOfVerts;             // size is the VERTEX struct * 3
		VertBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
		VertBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

		HLP_HANDLE_HRESULT(g_pDevice->CreateBuffer(&VertBufDesc, NULL, &g_pVBuffer));       // create the buffer

		U32 Stride = sizeof(VERTEX);
		U32 Offset = 0;
		g_pDevCon->IASetVertexBuffers(0, 1, &g_pVBuffer, &Stride, &Offset);
	}

	U0 UploadVbuf(FResource* pResources, FRenderState* pRenderState)
	{
		VERTEX* UploadVerts = (VERTEX*)malloc(pRenderState->NumVBuf * sizeof(VERTEX));
		assert(UploadVerts != NULL);

		I32 VBufIndex = 0;

		for (int i = 0; i < pRenderState->NumResources; i++)
		{
			if (pResources[i].Type == MESH)
			{
				for (int j = 0; j < pResources->pData.pMesh->NumVerts; j++)
				{
					UploadVerts[VBufIndex] = VERTEX{ pResources[i].pData.pMesh->Verts[j].x,pResources[i].pData.pMesh->Verts[j].y,pResources[i].pData.pMesh->Verts[j].z, D3DXCOLOR(0.9f, 0.3f, 0.0f, 1.0f) };
					VBufIndex++;
				}
			}
			else if (pResources[i].Type == POINTCLOUD)
			{
				for (int j = 0; j < pResources->pData.pPointcloud->NumVerts; j++)
				{
					UploadVerts[VBufIndex] = VERTEX{ pResources[i].pData.pPointcloud->Verts[j].x,pResources[i].pData.pPointcloud->Verts[j].y,pResources[i].pData.pPointcloud->Verts[j].z, D3DXCOLOR(0.9f, 0.3f, 0.0f, 1.0f) };
					VBufIndex++;
				}
			}
		}

		assert(VBufIndex == pRenderState->NumVBuf);

		// copy the vertices into the buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		HLP_HANDLE_HRESULT(g_pDevCon->Map(g_pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms));    // map the buffer
		memcpy(ms.pData, UploadVerts, sizeof(VERTEX) * pRenderState->NumVBuf);                 // copy the data
		g_pDevCon->Unmap(g_pVBuffer, NULL);                                      // unmap the buffer

		free(UploadVerts);
	}

	U0 UploadCbuf(FGameLogicState* pGLS)
	{
		ConstBuffer cbuf = { };
		cbuf.mvp = XMMathComputeMVP(&(pGLS->Cam));

		D3D11_MAPPED_SUBRESOURCE ms;
		HLP_HANDLE_HRESULT(g_pDevCon->Map(g_pCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms));
		memcpy(ms.pData, &cbuf, sizeof(ConstBuffer));
		g_pDevCon->Unmap(g_pCBuffer, 0);
	}


	U0 InitIndexBuffer(FResource* pResources, FRenderState* pRenderState)
	{
		U32 NumTotalIdx = 0;

		for (int i = 0; i < pRenderState->NumResources; i++)
		{
			if (pResources[i].Type == MESH)
			{
				NumTotalIdx += pResources->pData.pMesh->NumIdx;
			}
		}

		pRenderState->NumIBuf = NumTotalIdx;

		D3D11_BUFFER_DESC IndexBufferDesc;
		ZeroMemory(&IndexBufferDesc, sizeof(IndexBufferDesc));

		IndexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		IndexBufferDesc.ByteWidth = sizeof(U32) * NumTotalIdx;
		IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		IndexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		IndexBufferDesc.MiscFlags = 0;


		HLP_HANDLE_HRESULT(g_pDevice->CreateBuffer(&IndexBufferDesc, 0, &g_pIBuffer));

		g_pDevCon->IASetIndexBuffer(g_pIBuffer, DXGI_FORMAT_R32_UINT, 0);

	}

	U0 UploadIBuf(FResource* pResources, FRenderState* pRenderState)
	{
		U32* UploadIdx = (U32*)malloc(pRenderState->NumIBuf * sizeof(U32));
		assert(UploadIdx != NULL);

		I32 IBufIndex = 0;

		for (int i = 0; i < pRenderState->NumResources; i++)
		{
			if (pResources[i].Type == MESH)
			{
				for (int j = 0; j < pResources[i].pData.pMesh->NumIdx; j++)
				{
					UploadIdx[IBufIndex] = pResources[i].pData.pMesh->Idx[j];
					IBufIndex++;
				}
			}

		}

		assert(IBufIndex == pRenderState->NumIBuf);

		// copy the vertices into the buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		HLP_HANDLE_HRESULT(g_pDevCon->Map(g_pIBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms));    // map the buffer
		memcpy(ms.pData, UploadIdx, sizeof(U32) * pRenderState->NumIBuf);                 // copy the data
		g_pDevCon->Unmap(g_pIBuffer, NULL);                                      // unmap the buffer

		g_pDevCon->IASetIndexBuffer(g_pIBuffer, DXGI_FORMAT_R32_UINT, 0);
		free(UploadIdx);
	}

	// Cleanup
	U0 CleanUpD3D(U0)
	{
		// switch to windowed mode
		HLP_HANDLE_HRESULT(g_pSwapChain->SetFullscreenState(FALSE, NULL));

		// close and release all existing COM objects
		g_pLayout->Release();
		g_pVS->Release();
		g_pPS->Release();
		g_pVBuffer->Release();
		g_pIBuffer->Release();
		g_pDepthStencilView->Release();
		g_pDepthStencilBuffer->Release();
		g_pSwapChain->Release();
		g_pBackbuffer->Release();
		g_pDevice->Release();
		g_pDevCon->Release();
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	U0 DrawFrame(FResource* pResources, FRenderState* pRenderState)
	{
		// clear the back buffer to a deep blue
		g_pDevCon->ClearRenderTargetView(g_pBackbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));

		// clear depth stencil view
		g_pDevCon->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


		bool bIsTriList = true;
		U32 CurrentVBufIndex = 0;
		U32 CurrentIBufIndex = 0;

		g_pDevCon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (I32 i = 0; i < pRenderState->NumResources; i++)
		{
			// Set primitive topo
			if (pResources[i].Type == MESH && bIsTriList)
			{
#if 0
				g_pDevCon->DrawIndexed(
					pResources[i].pData.pMesh->NumIdx,
					CurrentIBufIndex,
					CurrentVBufIndex
				);
				CurrentIBufIndex += pResources[i].pData.pMesh->NumIdx;
				CurrentVBufIndex += pResources[i].pData.pMesh->NumVerts;
#else
				g_pDevCon->Draw(pResources[i].pData.pMesh->NumIdx, CurrentVBufIndex);
#endif
			}
			else if (pResources[i].Type == MESH && !bIsTriList)
			{
				g_pDevCon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				bIsTriList = true;
				g_pDevCon->DrawIndexed(
					pResources[i].pData.pMesh->NumIdx,
					CurrentIBufIndex,
					CurrentVBufIndex
				);
				CurrentIBufIndex += pResources[i].pData.pMesh->NumIdx;
				CurrentVBufIndex += pResources[i].pData.pMesh->NumVerts;
			}
			else if (pResources[i].Type == POINTCLOUD && !bIsTriList)
			{
				// correct type, correct topology
				g_pDevCon->Draw(
					pResources[i].pData.pPointcloud->NumVerts,
					CurrentVBufIndex 
				);
				CurrentVBufIndex += pResources[i].pData.pPointcloud->NumVerts;
			}
			else if (pResources[i].Type == POINTCLOUD && bIsTriList)
			{
				g_pDevCon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
				bIsTriList = false;
				g_pDevCon->Draw(
					pResources[i].pData.pPointcloud->NumVerts,
					CurrentVBufIndex
				);
				CurrentVBufIndex += pResources[i].pData.pPointcloud->NumVerts;
			}
		}

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// switch the back buffer and the front buffer
		HLP_HANDLE_HRESULT(g_pSwapChain->Present(0, 0));
	}
}