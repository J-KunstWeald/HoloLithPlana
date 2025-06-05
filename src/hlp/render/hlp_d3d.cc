#pragma once
#include "../hlp_state.hh"
#include "../hlp_math.hh"
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

#include "../app/hlp_util.hh"
// 1. Input Assembler(IA) Stage
// 2. Vertex Shader(VS) Stage
// 3. Hull Shader(HS) Stage
// 4. Tesselator Shader(TS) Stage
// 5. Domain Shader(DS) Stage
// 6. Geometry Shader(GS) Stage
// 7. Stream Output(SO) Stage
// 8. Rasterizer(RS) Stage
// 9. Pixel Shader(PS) Stage
// 10. Output Merger(OM) Stage

namespace hlp
{
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
		F32 nx;
		F32 ny;
		F32 nz;
	};

	// Error checking
	HRESULT g_hr;
	// Core
	IDXGISwapChain* g_pSwapChain;
	ID3D11Device* g_pDevice;
	ID3D11DeviceContext* g_pDevCon;
	ID3D11InputLayout* g_pLayout;
	ID3D11RenderTargetView* g_pBackbuffer;
	ID3D11DepthStencilView* g_pDepthStencilView;
	ID3D11Texture2D* g_pDepthStencilBuffer;
	// Buffer
	ID3D11Buffer* g_pVBuffer;
	ID3D11Buffer* g_pCBuffer;
	ID3D11Buffer* g_pIBuffer;
	// Shader
	ID3D11VertexShader* g_pVS;
	ID3D11PixelShader* g_pPS;

	DirectX::XMMATRIX g_VP;

	U0 InitSwpDevDevConDBuf(FAppState* pAppState)
	{
		//Describe our SwapChain Buffer
		DXGI_MODE_DESC bufferDesc;

		ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

		bufferDesc.Width = pAppState->Width;
		bufferDesc.Height = pAppState->Height;
		bufferDesc.RefreshRate.Numerator = pAppState->pRenderState->MaxFps;
		bufferDesc.RefreshRate.Denominator = 1;
		bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		//Describe our SwapChain
		DXGI_SWAP_CHAIN_DESC SwapChainDesc;

		ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
		
		SwapChainDesc.BufferDesc = bufferDesc;
		SwapChainDesc.SampleDesc.Count = 1;
		SwapChainDesc.SampleDesc.Quality = 0;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.BufferCount = 1;
		SwapChainDesc.OutputWindow = pAppState->hWnd;
		SwapChainDesc.Windowed = TRUE;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


		//Create our SwapChain
		g_hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
			D3D11_SDK_VERSION, &SwapChainDesc, &g_pSwapChain, &g_pDevice, NULL, &g_pDevCon);

		//Create our BackBuffer
		ID3D11Texture2D* BackBuffer;
		g_hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

		//Create our Render Target
		g_hr = g_pDevice->CreateRenderTargetView(BackBuffer, NULL, &g_pBackbuffer);
		BackBuffer->Release();

		//Describe our Depth/Stencil Buffer
		D3D11_TEXTURE2D_DESC DepthStencilDesc;

		DepthStencilDesc.Width = pAppState->Width;
		DepthStencilDesc.Height = pAppState->Height;
		DepthStencilDesc.MipLevels = 1;
		DepthStencilDesc.ArraySize = 1;
		DepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		DepthStencilDesc.SampleDesc.Count = 1;
		DepthStencilDesc.SampleDesc.Quality = 0;
		DepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		DepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		DepthStencilDesc.CPUAccessFlags = 0;
		DepthStencilDesc.MiscFlags = 0;
		
		//Create the Depth/Stencil View
		g_pDevice->CreateTexture2D(&DepthStencilDesc, NULL, &g_pDepthStencilBuffer);
		g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer, NULL, &g_pDepthStencilView);

		//Set our Render Target
		g_pDevCon->OMSetRenderTargets(1, &g_pBackbuffer, g_pDepthStencilView);
	}

	U0 InitShaders()
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

		D3D11_INPUT_ELEMENT_DESC VertBufInpElemDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		//Create the Input Layout
		g_hr = g_pDevice->CreateInputLayout(VertBufInpElemDesc, 3, VS->GetBufferPointer(),
			VS->GetBufferSize(), &g_pLayout);

		//Set the Input Layout
		g_pDevCon->IASetInputLayout(g_pLayout);

		//Set Primitive Topology
		g_pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	U0 InitViewPort(FAppState* pAppState)
	{
		D3D11_VIEWPORT Viewport;
		ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));

		Viewport.TopLeftX = 0;
		Viewport.TopLeftY = 0;
		Viewport.Width = pAppState->Width;
		Viewport.Height = pAppState->Height;
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;

		//Set the Viewport
		g_pDevCon->RSSetViewports(1, &Viewport);
	}

	U0 UpdateIBuf(FAppState* pAppState)
	{
		U32* pIndices = (U32*)malloc(sizeof(U32) * pAppState->pRenderState->TotNumIdx);
		U32 CurrentIndex = 0;
		for (int i = 0; i < pAppState->pSceneState->NumGeometries; i++)
		{
			if (pAppState->pSceneState->Geometries[i].Type == MESH)
			{
				for (int j = 0; j < pAppState->pSceneState->Geometries[i].pData.pMesh->NumIdx; j++)
				{
					pIndices[CurrentIndex] = pAppState->pSceneState->Geometries[i].pData.pMesh->Idx[j];
					CurrentIndex++;
				}
				//PrintFMesh(pAppState->pSceneState->Geometries[i].pData.pMesh);
			}
		}
		HLP_ASSERT(CurrentIndex == pAppState->pRenderState->TotNumIdx);

		D3D11_BUFFER_DESC IndexBufferDesc;
		ZeroMemory(&IndexBufferDesc, sizeof(IndexBufferDesc));

		IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		IndexBufferDesc.ByteWidth = sizeof(U32) * pAppState->pRenderState->TotNumIdx;
		IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		IndexBufferDesc.CPUAccessFlags = 0;
		IndexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA Data;
		ZeroMemory(&Data, sizeof(Data));

		Data.pSysMem = pIndices;

		g_pDevice->CreateBuffer(&IndexBufferDesc, &Data, &g_pIBuffer);

		g_pDevCon->IASetIndexBuffer(g_pIBuffer, DXGI_FORMAT_R32_UINT, 0);

		free(pIndices);
	}

	U0 UpdateVBuf(FAppState* pAppState)
	{
		FVertex* pVerts = (FVertex*)malloc(sizeof(FVertex) * pAppState->pRenderState->TotNumVerts);
		U32 CurrentIndex = 0;
		for (int i = 0; i < pAppState->pSceneState->NumGeometries; i++)
		{
			if (pAppState->pSceneState->Geometries[i].Type == MESH)
			{
				for (int j = 0; j < pAppState->pSceneState->Geometries[i].pData.pMesh->NumVerts; j++)
				{
					pVerts[CurrentIndex] = FVertex{
					pAppState->pSceneState->Geometries[i].pData.pMesh->Verts[j].x,
					pAppState->pSceneState->Geometries[i].pData.pMesh->Verts[j].y,
					pAppState->pSceneState->Geometries[i].pData.pMesh->Verts[j].z,
					pAppState->pSceneState->Geometries[i].pData.pMesh->Color.x,
					pAppState->pSceneState->Geometries[i].pData.pMesh->Color.y,
					pAppState->pSceneState->Geometries[i].pData.pMesh->Color.z,
					pAppState->pSceneState->Geometries[i].pData.pMesh->Color.w,
					pAppState->pSceneState->Geometries[i].pData.pMesh->Normals[j].x,
					pAppState->pSceneState->Geometries[i].pData.pMesh->Normals[j].y,
					pAppState->pSceneState->Geometries[i].pData.pMesh->Normals[j].z,
					};
						
					CurrentIndex++;
				}
			}
		}
		HLP_ASSERT(CurrentIndex == pAppState->pRenderState->TotNumVerts);

		D3D11_BUFFER_DESC VertexBufferDesc;
		ZeroMemory(&VertexBufferDesc, sizeof(VertexBufferDesc));

		VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		VertexBufferDesc.ByteWidth = sizeof(FVertex) * pAppState->pRenderState->TotNumVerts;
		VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		VertexBufferDesc.CPUAccessFlags = 0;
		VertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA Data;
		ZeroMemory(&Data, sizeof(Data));

		Data.pSysMem = pVerts;

		g_hr = g_pDevice->CreateBuffer(&VertexBufferDesc, &Data, &g_pVBuffer);

		//Set the vertex buffer
		UINT Stride = sizeof(FVertex);
		UINT Offset = 0;
		g_pDevCon->IASetVertexBuffers(0, 1, &g_pVBuffer, &Stride, &Offset);

		free(pVerts);
	}

	U0 InitVertLayout(FAppState* pAppState)
	{

	}

	U0 InitCBuf(FAppState* pAppState)
	{
		//FConstBuffer
		D3D11_BUFFER_DESC ConstBufferDesc;
		ZeroMemory(&ConstBufferDesc, sizeof(D3D11_BUFFER_DESC));

		ConstBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		ConstBufferDesc.ByteWidth = sizeof(FConstBuffer);
		ConstBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ConstBufferDesc.CPUAccessFlags = 0;
		ConstBufferDesc.MiscFlags = 0;

		g_hr = g_pDevice->CreateBuffer(&ConstBufferDesc, NULL, &g_pCBuffer);
	}

	U0 Draw(FAppState* pAppState)
	{
		if (pAppState->pRenderState->bNewGeometry)
		{
			UpdateVBuf(pAppState);
			UpdateIBuf(pAppState);
			pAppState->pRenderState->bNewGeometry = false;
		}

		//Clear our backbuffer
		float bgColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		g_pDevCon->ClearRenderTargetView(g_pBackbuffer, bgColor);

		//Refresh the Depth/Stencil view
		g_pDevCon->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// calc VP
		g_VP = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f) * DirectX::XMMatrixPerspectiveFovLH(
			(0.4f * 3.14f),
			pAppState->pSceneState->Cam.AspectRatio,
			pAppState->pSceneState->Cam.NearPlane,
			pAppState->pSceneState->Cam.FarPlane
			) 
			* 
			DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(pAppState->pSceneState->Cam.CameraPosition.x, pAppState->pSceneState->Cam.CameraPosition.y, pAppState->pSceneState->Cam.CameraPosition.z,0.0f),
			DirectX::XMVectorSet(pAppState->pSceneState->Cam.CameraForward.x, pAppState->pSceneState->Cam.CameraForward.y, pAppState->pSceneState->Cam.CameraForward.z, 0.0f),
				DirectX::XMVectorSet(pAppState->pSceneState->Cam.CameraUp.x, pAppState->pSceneState->Cam.CameraUp.y, pAppState->pSceneState->Cam.CameraUp.z, 0.0f)
			);

		U32 CurrentVBufIdx = 0;
		U32 CurrentIBufIdx = 0;
		FConstBuffer cBuf = { };
		for (int i = 0; i < pAppState->pSceneState->NumGeometries; i++)
		{
			cBuf.mvp = DirectX::XMMatrixTranspose(g_VP);
			g_pDevCon->UpdateSubresource(g_pCBuffer, 0, NULL, &cBuf, 0, 0);
			g_pDevCon->VSSetConstantBuffers(0, 1, &g_pCBuffer);

			g_pDevCon->DrawIndexed(pAppState->pSceneState->Geometries[i].pData.pMesh->NumIdx, CurrentIBufIdx, CurrentVBufIdx);
			CurrentIBufIdx += pAppState->pSceneState->Geometries[i].pData.pMesh->NumIdx;
			CurrentVBufIdx += pAppState->pSceneState->Geometries[i].pData.pMesh->NumVerts;
		}

		g_pSwapChain->Present(0, 0);
	}

	U0 InitD3D(FAppState* pAppState)
	{
		InitSwpDevDevConDBuf(pAppState);
		InitShaders();
		InitViewPort(pAppState);
		InitCBuf(pAppState);
	}

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
		g_pCBuffer->Release();
		g_pDepthStencilView->Release();
		g_pDepthStencilBuffer->Release();
		g_pSwapChain->Release();
		g_pBackbuffer->Release();
		g_pDevice->Release();
		g_pDevCon->Release();
		//ImGui_ImplDX11_Shutdown();
		//ImGui_ImplWin32_Shutdown();
		//ImGui::DestroyContext();
	}
}