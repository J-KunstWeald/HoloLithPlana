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

#include <wincodec.h> 
#include <wrl/client.h>  

#include <memory>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

#include "../app/hlp_util.hh"
#include "hlp_imgui.hh"

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
		F32 u;
		F32 v;
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
	// Rasterizer States
	ID3D11RasterizerState* g_pRSWF;
	ID3D11RasterizerState* g_pRSS;
	// Texture things
	ID3D11ShaderResourceView* g_pTextureSRV;
	ID3D11SamplerState* g_pSampler;

	DirectX::XMMATRIX g_VP;

#define SAFE_RELEASE(p) if(p){ (p)->Release(); (p) = nullptr; }

	HRESULT LoadTextureFromFileWIC(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		const wchar_t* filename,
		ID3D11ShaderResourceView** textureView
	) {
		IWICImagingFactory* wicFactory = nullptr;
		IWICBitmapDecoder* decoder = nullptr;
		IWICBitmapFrameDecode* frame = nullptr;
		IWICFormatConverter* converter = nullptr;
		ID3D11Texture2D* texture = nullptr;

		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr)) return hr;

		hr = CoCreateInstance(
			CLSID_WICImagingFactory, nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory)
		);
		if (FAILED(hr)) return hr;

		hr = wicFactory->CreateDecoderFromFilename(
			filename,
			nullptr,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&decoder
		);
		if (FAILED(hr))
		{
			SAFE_RELEASE(texture);
			SAFE_RELEASE(converter);
			SAFE_RELEASE(frame);
			SAFE_RELEASE(decoder);
			SAFE_RELEASE(wicFactory);
			return hr;
		}

		hr = decoder->GetFrame(0, &frame);
		if (FAILED(hr))
		{
			SAFE_RELEASE(texture);
			SAFE_RELEASE(converter);
			SAFE_RELEASE(frame);
			SAFE_RELEASE(decoder);
			SAFE_RELEASE(wicFactory);
			return hr;
		}

		hr = wicFactory->CreateFormatConverter(&converter);
		if (FAILED(hr))
		{
			SAFE_RELEASE(texture);
			SAFE_RELEASE(converter);
			SAFE_RELEASE(frame);
			SAFE_RELEASE(decoder);
			SAFE_RELEASE(wicFactory);
			return hr;
		}

		hr = converter->Initialize(
			frame,
			GUID_WICPixelFormat32bppRGBA, // Convert to DXGI_FORMAT_R8G8B8A8_UNORM
			WICBitmapDitherTypeNone,
			nullptr,
			0.0,
			WICBitmapPaletteTypeCustom
		);
		if (FAILED(hr))
		{
			SAFE_RELEASE(texture);
			SAFE_RELEASE(converter);
			SAFE_RELEASE(frame);
			SAFE_RELEASE(decoder);
			SAFE_RELEASE(wicFactory);
			return hr;
		}

		UINT width, height;
		hr = converter->GetSize(&width, &height);
		if (FAILED(hr))
		{
			SAFE_RELEASE(texture);
			SAFE_RELEASE(converter);
			SAFE_RELEASE(frame);
			SAFE_RELEASE(decoder);
			SAFE_RELEASE(wicFactory);
			return hr;
		}

		std::unique_ptr<BYTE[]> pixels(new BYTE[width * height * 4]);
		hr = converter->CopyPixels(
			nullptr,
			width * 4,
			width * height * 4,
			pixels.get()
		);
		if (FAILED(hr))
		{
			SAFE_RELEASE(texture);
			SAFE_RELEASE(converter);
			SAFE_RELEASE(frame);
			SAFE_RELEASE(decoder);
			SAFE_RELEASE(wicFactory);
			return hr;
		}

		// Describe texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = pixels.get();
		initData.SysMemPitch = width * 4;

		hr = device->CreateTexture2D(&desc, &initData, &texture);
		if (FAILED(hr))
		{
			SAFE_RELEASE(texture);
			SAFE_RELEASE(converter);
			SAFE_RELEASE(frame);
			SAFE_RELEASE(decoder);
			SAFE_RELEASE(wicFactory);
			return hr;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		hr = device->CreateShaderResourceView(texture, &srvDesc, textureView);

		return hr;
	}


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
		HLP_HANDLE_HRESULT(g_hr);
		//Create our BackBuffer
		ID3D11Texture2D* BackBuffer;
		g_hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
		HLP_HANDLE_HRESULT(g_hr);
		//Create our Render Target
		g_hr = g_pDevice->CreateRenderTargetView(BackBuffer, NULL, &g_pBackbuffer);
		HLP_HANDLE_HRESULT(g_hr);
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

	U0 InitRasterStates()
	{
		D3D11_RASTERIZER_DESC WFDS;
		ZeroMemory(&WFDS, sizeof(D3D11_RASTERIZER_DESC));
		WFDS.FillMode = D3D11_FILL_WIREFRAME;
		WFDS.CullMode = D3D11_CULL_NONE;
		g_hr = g_pDevice->CreateRasterizerState(&WFDS, &g_pRSWF);
		HLP_HANDLE_HRESULT(g_hr);

		D3D11_RASTERIZER_DESC SDS;
		ZeroMemory(&SDS, sizeof(D3D11_RASTERIZER_DESC));
		SDS.FillMode = D3D11_FILL_SOLID;
		SDS.CullMode = D3D11_CULL_BACK;
		g_hr = g_pDevice->CreateRasterizerState(&SDS, &g_pRSS);
		HLP_HANDLE_HRESULT(g_hr);

		g_pDevCon->RSSetState(g_pRSS);
	}

	U0 InitSamplerAndTexture(FAppState* pAS)
	{
		g_hr = LoadTextureFromFileWIC(g_pDevice,g_pDevCon,HLP_TEXTURE_LOCATION,&g_pTextureSRV);
		HLP_HANDLE_HRESULT(g_hr);

		D3D11_SAMPLER_DESC SampDesc;
		ZeroMemory(&SampDesc, sizeof(SampDesc));
		SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		SampDesc.MinLOD = 0;
		SampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		g_hr = g_pDevice->CreateSamplerState(&SampDesc, &g_pSampler);
		HLP_HANDLE_HRESULT(g_hr);

		g_pDevCon->PSSetShaderResources(0, 1, &g_pTextureSRV);
		g_pDevCon->PSSetSamplers(0, 1, &g_pSampler);
	}

	U0 ChangeRasterizerState(FAppState* pAppState)
	{
		if (pAppState->pRenderState->bRenderSolid)
		{
			g_pDevCon->RSSetState(g_pRSS);
			pAppState->pRenderState->bRenderSolid = false;
		}
		else if (pAppState->pRenderState->bRenderWireframe)
		{
			g_pDevCon->RSSetState(g_pRSWF);
			pAppState->pRenderState->bRenderWireframe = false;
		}

		pAppState->pRenderState->bChangeRasterizer = false;
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
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		//Create the Input Layout
		g_hr = g_pDevice->CreateInputLayout(VertBufInpElemDesc, 3, VS->GetBufferPointer(),
			VS->GetBufferSize(), &g_pLayout);
		HLP_HANDLE_HRESULT(g_hr);

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
					0.8f,
					0.9f,
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
		HLP_HANDLE_HRESULT(g_hr);

		//Set the vertex buffer
		UINT Stride = sizeof(FVertex);
		UINT Offset = 0;
		g_pDevCon->IASetVertexBuffers(0, 1, &g_pVBuffer, &Stride, &Offset);
		HLP_HANDLE_HRESULT(g_hr);

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
		HLP_HANDLE_HRESULT(g_hr);
	}

	U0 Draw(FAppState* pAppState)
	{
		if (pAppState->pRenderState->bChangeRasterizer)
		{
			ChangeRasterizerState(pAppState);
		}

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
		g_VP = DirectX::XMMatrixLookAtLH(
				DirectX::XMVectorSet(pAppState->pSceneState->Cam.CameraPosition.x, pAppState->pSceneState->Cam.CameraPosition.y, pAppState->pSceneState->Cam.CameraPosition.z,0.0f),
				DirectX::XMVectorSet(pAppState->pSceneState->Cam.CameraForward.x, pAppState->pSceneState->Cam.CameraForward.y, pAppState->pSceneState->Cam.CameraForward.z, 0.0f),
				DirectX::XMVectorSet(pAppState->pSceneState->Cam.CameraUp.x, pAppState->pSceneState->Cam.CameraUp.y, pAppState->pSceneState->Cam.CameraUp.z, 0.0f)
				)
				*
				DirectX::XMMatrixPerspectiveFovLH(
				HLP_TO_RADIAN(pAppState->pSceneState->Cam.HorizontalFOVDeg),
				pAppState->pSceneState->Cam.AspectRatio,
				pAppState->pSceneState->Cam.NearPlane,
				pAppState->pSceneState->Cam.FarPlane
				);

		U32 CurrentVBufIdx = 0;
		U32 CurrentIBufIdx = 0;
		FConstBuffer cBuf = { };
		for (int i = 0; i < pAppState->pSceneState->NumGeometries; i++)
		{
			if (i % 2 == 0)
			{
				cBuf.mvp = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(0.15f, 0.1f, 2.0f) * g_VP);
			}
			else
			{
				cBuf.mvp = DirectX::XMMatrixTranspose(g_VP);
			}
			
			g_pDevCon->UpdateSubresource(g_pCBuffer, 0, NULL, &cBuf, 0, 0);
			g_pDevCon->VSSetConstantBuffers(0, 1, &g_pCBuffer);

			g_pDevCon->DrawIndexed(pAppState->pSceneState->Geometries[i].pData.pMesh->NumIdx, CurrentIBufIdx, CurrentVBufIdx);
			CurrentIBufIdx += pAppState->pSceneState->Geometries[i].pData.pMesh->NumIdx;
			CurrentVBufIdx += pAppState->pSceneState->Geometries[i].pData.pMesh->NumVerts;
		}

		// Imgui
		DrawImguiFrame();

		g_pSwapChain->Present(0, 0);
	}

	U0 InitD3D(FAppState* pAppState)
	{
		InitSwpDevDevConDBuf(pAppState);
		InitShaders();
		InitViewPort(pAppState);
		InitRasterStates();
		InitCBuf(pAppState);
		InitSamplerAndTexture(pAppState);
		InitImgui(&(pAppState->hWnd), g_pDevice, g_pDevCon);
	}

	U0 CleanUpD3D(U0)
	{
		// switch to windowed mode
		HLP_HANDLE_HRESULT(g_pSwapChain->SetFullscreenState(FALSE, NULL));

		// close and release all existing COM objects
		g_pRSS->Release();
		g_pRSWF->Release();
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
		CleanUpImgui();
	}
}