extern Simulation simulation;

namespace Renderer
{
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D11.lib") 

	void DrawSceneByCellType();
	void DrawCellsByAttachmentStrength();
	void DrawAllCells();
	void DrawUI();

	IDXGISwapChain* swapChain = nullptr;
	ID3D11Device* mainDevice = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ID3D11RenderTargetView* renderTargetView = nullptr;
	ID3D11Texture2D* depthStencilBuffer = nullptr;
	ID3D11DepthStencilState* depthStencilState = nullptr;
	ID3D11DepthStencilView* depthStencilView = nullptr;
	ID3D11RasterizerState* rasterState = nullptr;

	ID3D11VertexShader* uiVertexShader = nullptr;
	ID3D11InputLayout* uiInputLayout = nullptr;
	ID3D11PixelShader* uiPixelShader = nullptr;
	ID3D11Buffer* meshVertexBuffer = nullptr;
	ID3D11Buffer* meshIndexBuffer = nullptr;
	ID3D11Buffer* matrixBuffer = nullptr;
	ID3D11Buffer* psCBuffer = nullptr;

	float clearColour[4];
	UINT32 videoCardMemory = 0;
	UINT32 numUIQuads = 0;

	UINT width;
	UINT height;

	float frame;
	
	const int batchSize = 4096;
	int m_numVerts;

	float* matrixScratchBuffer;
	
	namespace Colours
	{
		enum Values
		{
			Black,
			Grey,
			LightGrey,
			Transparent,
			Count
		};
	}

	bool LoadVertexShaderAndBuildInputLayout(std::string filename, ID3D11VertexShader** vertexShader, const D3D11_INPUT_ELEMENT_DESC *inputElementDescs, const INT32 numInputElelments, ID3D11InputLayout** inputLayout)
	{
		bool overallSuccess = true;

		INT32 length;
		bool fileReadSuccessful;

		char* buffer = Utils::ReadBinaryFile(filename, length, fileReadSuccessful);

		if(fileReadSuccessful)
		{

			HRESULT hr = mainDevice->CreateVertexShader(buffer, length, nullptr, vertexShader);

			if(hr != S_OK)
			{
				overallSuccess = false;
			}
		}
		else
		{
			overallSuccess = false;
		}

		if(overallSuccess)
		{
			HRESULT hr = mainDevice->CreateInputLayout(inputElementDescs, numInputElelments, buffer, length, inputLayout);

			if(hr != S_OK)
			{
				overallSuccess = false;
			}
		}

		delete[] buffer;

		return overallSuccess;
	}

	bool LoadPixelShader(std::string filename, ID3D11PixelShader** pixelShader)
	{
		bool overallSuccess = true;

		INT32 length;
		bool fileReadSuccessful;

		char* buffer = Utils::ReadBinaryFile(filename, length, fileReadSuccessful);

		if(fileReadSuccessful == true)
		{

			HRESULT hr = mainDevice->CreatePixelShader(buffer, length, nullptr, pixelShader);

			if(hr != S_OK)
			{
				overallSuccess = false;
			}
		}
		else
		{
			overallSuccess = false;
		}

		delete[] buffer;

		return overallSuccess;
	}

	bool InitD3DDevice(HWND hwnd)
	{
		HRESULT result;
		IDXGIFactory* factory;
		IDXGIAdapter* adapter;
		IDXGIOutput* adapterOutput;
		ID3D11Texture2D* backBufferPtr;
		unsigned int numModes, i, numerator, denominator;
		DXGI_MODE_DESC* displayModeList;
		DXGI_ADAPTER_DESC adapterDesc;
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		D3D_FEATURE_LEVEL featureLevel;
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		D3D11_RASTERIZER_DESC rasterDesc;
		D3D11_VIEWPORT viewport;


		result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
		if(FAILED(result))
		{
			return false;
		}

		result = factory->EnumAdapters(0, &adapter);
		if(FAILED(result))
		{
			return false;
		}

		result = adapter->EnumOutputs(0, &adapterOutput);
		if(FAILED(result))
		{
			return false;
		}

		result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
		if(FAILED(result))
		{
			return false;
		}

		displayModeList = new DXGI_MODE_DESC[numModes];
		if(!displayModeList)
		{
			return false;
		}

		result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
		if(FAILED(result))
		{
			return false;
		}

		for(i=0; i<numModes; i++)
		{
			if(displayModeList[i].Width == width)
			{
				if(displayModeList[i].Height == height)
				{
					numerator = displayModeList[i].RefreshRate.Numerator;
					denominator = displayModeList[i].RefreshRate.Denominator;
				}
			}
		}

		result = adapter->GetDesc(&adapterDesc);
		if(FAILED(result))
		{
			return false;
		}

		videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

		/*error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
		if(error != 0)
		{
		return false;
		}*/

		delete [] displayModeList;
		displayModeList = 0;

		adapterOutput->Release();
		adapterOutput = 0;

		adapter->Release();
		adapter = 0;

		factory->Release();
		factory = 0;

		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

		swapChainDesc.BufferCount = 1;

		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;

		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		/*if(m_vsync_enabled)
		{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
		}
		else*/
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		swapChainDesc.OutputWindow = hwnd;

		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		/*if(fullscreen)
		{
		swapChainDesc.Windowed = false;
		}
		else*/
		{
			swapChainDesc.Windowed = true;
		}

		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		swapChainDesc.Flags = 0;

		featureLevel = D3D_FEATURE_LEVEL_11_0;
		

		result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, &featureLevel, 1, 
		//result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, &featureLevel, 1, 
			D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &mainDevice, NULL, &deviceContext);
		if(FAILED(result))
		{
			return false;
		}

		result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
		if(FAILED(result))
		{
			return false;
		}

		result = mainDevice->CreateRenderTargetView(backBufferPtr, NULL, &renderTargetView);
		if(FAILED(result))
		{
			return false;
		}

		backBufferPtr->Release();
		backBufferPtr = 0;

		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		depthBufferDesc.Width = width;
		depthBufferDesc.Height = height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		result = mainDevice->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);
		if(FAILED(result))
		{
			return false;
		}

		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

		depthStencilDesc.StencilEnable = false;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		result = mainDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
		if(FAILED(result))
		{
			return false;
		}

		deviceContext->OMSetDepthStencilState(depthStencilState, 1);

		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = mainDevice->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
		if(FAILED(result))
		{
			return false;
		}

		deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = true;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;

		result = mainDevice->CreateRasterizerState(&rasterDesc, &rasterState);
		if(FAILED(result))
		{
			return false;
		}

		deviceContext->RSSetState(rasterState);

		viewport.Width = (float)width;
		viewport.Height = (float)height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		deviceContext->RSSetViewports(1, &viewport);

		return true;
	}

	void Init(HWND hwnd, UINT screenWidth, UINT screenHeight)
	{
		frame = 0;

		clearColour[0] = 0.0f;
		clearColour[1] = 0.0f;
		clearColour[2] = 1.0f;
		clearColour[3] = 1.0f;

		width = screenWidth;
		height = screenHeight;

		matrixScratchBuffer = new float[16 * batchSize];

		// There is an argument to be made for rolling all of these functions into this main
		// Init.  It would gauruntee that they are only called here and in the correct order.
		// On the other hand it would make jumping to the specific piece of code much harder.
		InitD3DDevice(hwnd);

		CreateInstancingMatrixBuffer(mainDevice, &matrixBuffer, batchSize);

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, 
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, 
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, 
			D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, 
			D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, 
			D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, 
			D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};

		LoadVertexShaderAndBuildInputLayout("SimpleVS.cso", &uiVertexShader, layout, 6, &uiInputLayout);
		LoadPixelShader("SimplePS.cso", &uiPixelShader);
		LoadMeshBuffersFromFile(mainDevice, &meshVertexBuffer, &meshIndexBuffer,"crypt.obj", m_numVerts);

		float initCol[] = {1.0f, 1.0f, 0.0f, 1.0f};

		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = 16;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &initCol;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		mainDevice->CreateBuffer( &cbDesc, &initData, 
								 &psCBuffer );

		deviceContext->PSSetConstantBuffers( 0, 1, &psCBuffer );
	}

	void ReportLiveObjects()
	{
		ID3D11Debug* debugDevice;

		HRESULT hr = mainDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&debugDevice);

		if(hr == S_OK)
		{
			hr = debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

			debugDevice->Release();
		}
	}

	void CleanUp()
	{
		swapChain->Release();
		deviceContext->Release();
		renderTargetView->Release();
		rasterState->Release();
		depthStencilBuffer->Release();
		depthStencilState->Release();
		depthStencilView->Release();
		uiVertexShader->Release();
		uiPixelShader->Release();
		uiInputLayout->Release();
		meshIndexBuffer->Release();
		meshVertexBuffer->Release();
		matrixBuffer->Release();
		psCBuffer->Release();

		ReportLiveObjects();
		mainDevice->Release();

		delete[] matrixScratchBuffer;
	}



	void Draw()
	{
		deviceContext->ClearRenderTargetView(renderTargetView, clearColour);
		deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		
		//DrawSceneByCellType();
		//DrawCellsByAttachmentStrength();
		DrawAllCells();

		swapChain->Present(0, 0);
	}

	void DrawBatch(int numInBatch)
	{
		D3D11_MAPPED_SUBRESOURCE matrix;
		deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &matrix);

		memcpy(matrix.pData, matrixScratchBuffer, sizeof(float) * 16 * numInBatch);

		deviceContext->Unmap(matrixBuffer, 0);

		UINT32 strides[2];
		UINT32 offsets[2];
		ID3D11Buffer* bufferPointers[2];

		strides[0] = sizeof(UINT32) * 8;
		strides[1] = sizeof(float) * 16; 

		offsets[0] = 0;
		offsets[1] = 0;

		bufferPointers[0] = meshVertexBuffer;
		bufferPointers[1] = matrixBuffer;

		deviceContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
		deviceContext->IASetIndexBuffer(meshIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		deviceContext->IASetInputLayout(uiInputLayout);
		deviceContext->VSSetShader(uiVertexShader, nullptr, 0);
		deviceContext->PSSetShader(uiPixelShader, nullptr, 0);

		deviceContext->DrawIndexedInstanced(m_numVerts, numInBatch, 0, 0, 0);
	}

	void DrawCellsInBounds(float upper, float lower, float colour[], bool drawQuiescentCells)
	{
		D3D11_MAPPED_SUBRESOURCE mappedColour;
		deviceContext->Map(psCBuffer,0, D3D11_MAP_WRITE_DISCARD, 0, &mappedColour);

		memcpy(mappedColour.pData, colour, 16);

		deviceContext->Unmap(psCBuffer, 0);

		float zDist = 10000.1f;
		float height = -3000.0f;

		DirectX::FXMVECTOR camPos = DirectX::XMVectorSet(0.0f, height, zDist, 0.0f );
		DirectX::FXMVECTOR camLookAt = DirectX::XMVectorSet(0.0f, height, 0.0f, 0.0f );
		DirectX::FXMVECTOR camUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f );

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(camPos, camLookAt, camUp);
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(0.75f, 1280.0f / 720.0f, zDist - 1000.0f, zDist + 1000.0f);

		proj = DirectX::XMMatrixMultiply(view, proj);

		
		
		int numInBatch = 0;
/*		for(int i = 0; i< Simulation::crypts.size(); i++)
		{
			for(int col = 0; col < (int)Simulation::crypts[i]->m_grid.m_columns.size(); col++)
			{
				std::vector<CellBox>& column = Simulation::crypts[i]->m_grid.m_columns[col];
				for(int row = 0; row < (int)column.size(); row++)
				{
					CellBox& box = column[row];


					for(int cell = 0; cell < (int)box.m_positions.size(); cell++)
					{
						Vector3D& vec = box.m_positions[cell];
						bool quiescent = box.m_cycleStages[cell] == CellCycleStages::G0;

						if(vec.y < upper && vec.y > lower && quiescent == drawQuiescentCells)
						{
							DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(vec.x, vec.y, vec.z);
							DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(50.0f, 50.0f, 50.0f);

							world = DirectX::XMMatrixMultiply(scale, world);
							world = DirectX::XMMatrixMultiply(world, proj);

							DirectX::XMFLOAT4X4 mat;
							DirectX::XMStoreFloat4x4(&mat, world);

							memcpy(matrixScratchBuffer + 16 * numInBatch, &world, 16 * sizeof(float));
							numInBatch++;

							if(numInBatch == batchSize)
							{
								DrawBatch(numInBatch);
								numInBatch = 0;
							} 
						}
					}
				}
			}
		}*/
		DrawBatch(numInBatch);
	}

	void DrawCellsByAttachmentStrength()
	{
		frame++;

		float scaleFactor = 1000.0f; 
		float zDist = 10010.1f / scaleFactor;
		float height = 10000.0f / scaleFactor;
		float cellScale = 100.0f / scaleFactor;

		DirectX::FXMVECTOR camPos = DirectX::XMVectorSet(0.0f, height, zDist, 0.0f );
		DirectX::FXMVECTOR camLookAt = DirectX::XMVectorSet(0.0f, -500.0f / scaleFactor, 0.0f, 0.0f );
		DirectX::FXMVECTOR camUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f );

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(camPos, camLookAt, camUp);
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(0.75f, 1280.0f / 720.0f, zDist - 10000.0f / scaleFactor, zDist + 10000.0f / scaleFactor);

		proj = DirectX::XMMatrixMultiply(view, proj);

		float colour[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		D3D11_MAPPED_SUBRESOURCE mappedColour;
		deviceContext->Map(psCBuffer,0, D3D11_MAP_WRITE_DISCARD, 0, &mappedColour);

		memcpy(mappedColour.pData, colour, 16);

		deviceContext->Unmap(psCBuffer, 0);
		
		int numInBatch = 0;
		/*
		for(int i = 0; i< Simulation::crypts.size(); i++)
		{
			for(int col = 0; col < (int)Simulation::crypts[i]->m_grid.m_columns.size(); col++)
			{
				std::vector<CellBox>& column = Simulation::crypts[i]->m_grid.m_columns[col];
				for(int row = 0; row < (int)column.size(); row++)
				{
					CellBox& box = column[row];
					for(int cell = 0; cell < box.m_positions.size(); cell++)
					{
						Vector3D& vec = box.m_positions[cell];

						if(box.m_mutations[cell].mutateAttachment == false && box.m_mutations[cell].mutateCellForces == false && box.m_mutations[cell].mutateQuiecence == false)
						{
							DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(vec.x / scaleFactor, vec.y / scaleFactor, vec.z / scaleFactor);
							DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(cellScale, cellScale, cellScale);

							world = DirectX::XMMatrixMultiply(scale, world);
							world = DirectX::XMMatrixMultiply(world, proj);

							DirectX::XMFLOAT4X4 mat;
							DirectX::XMStoreFloat4x4(&mat, world);

							memcpy(matrixScratchBuffer + 16 * numInBatch, &world, 16 * sizeof(float));
							numInBatch++;

							if(numInBatch == batchSize)
							{
								DrawBatch(numInBatch);
								numInBatch = 0;
							} 
						}
					}
				}
			}
		}*/
		DrawBatch(numInBatch);

		colour[0] = 0.0f;
		colour[1] = 0.5f;
		deviceContext->Map(psCBuffer,0, D3D11_MAP_WRITE_DISCARD, 0, &mappedColour);

		memcpy(mappedColour.pData, colour, 16);

		deviceContext->Unmap(psCBuffer, 0);
		
		numInBatch = 0;
		/*
		for(int i = 0; i< Simulation::crypts.size(); i++)
		{
			for(int col = 0; col < (int)Simulation::crypts[i]->m_grid.m_columns.size(); col++)
			{
				std::vector<CellBox>& column = Simulation::crypts[i]->m_grid.m_columns[col];
				for(int row = 0; row < (int)column.size(); row++)
				{
					CellBox& box = column[row];
					for(int cell = 0; cell < box.m_positions.size(); cell++)
					{
						Vector3D& vec = box.m_positions[cell];

						if(box.m_mutations[cell].mutateAttachment == true || box.m_mutations[cell].mutateCellForces == true || box.m_mutations[cell].mutateQuiecence == true)
						{
							DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(vec.x / scaleFactor, vec.y / scaleFactor, vec.z / scaleFactor);
							DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(cellScale, cellScale, cellScale);

							world = DirectX::XMMatrixMultiply(scale, world);
							world = DirectX::XMMatrixMultiply(world, proj);

							DirectX::XMFLOAT4X4 mat;
							DirectX::XMStoreFloat4x4(&mat, world);

							memcpy(matrixScratchBuffer + 16 * numInBatch, &world, 16 * sizeof(float));
							numInBatch++;

							if(numInBatch == batchSize)
							{
								DrawBatch(numInBatch);
								numInBatch = 0;
							} 
						}
					}
				}
			}
		}*/
		DrawBatch(numInBatch);
	}

	void DrawAllCells()
	{
		frame++;

		float zDist = 100.1f;
		float height = 50.0f;

		DirectX::FXMVECTOR camPos = DirectX::XMVectorSet(0.0f, height, zDist, 0.0f );
		DirectX::FXMVECTOR camLookAt = DirectX::XMVectorSet(100.0f, 0.0f, 100.0f, 0.0f );
		DirectX::FXMVECTOR camUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f );

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(camPos, camLookAt, camUp);
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(0.75f, 1280.0f / 720.0f, zDist - 100.0f, zDist + 100.0f);

		proj = DirectX::XMMatrixMultiply(view, proj);

		float colour[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
		D3D11_MAPPED_SUBRESOURCE mappedColour;
		deviceContext->Map(psCBuffer,0, D3D11_MAP_WRITE_DISCARD, 0, &mappedColour);

		memcpy(mappedColour.pData, colour, 16);

		deviceContext->Unmap(psCBuffer, 0);

		int numInBatch = 0;
		/*
		for(int i = 0; i< Simulation::crypts.size(); i++)
			{*/
			for(int col = 0; col < (int)::simulation.m_grid.m_columns.size(); col++)
			{
				std::vector<CryptBox>& column = ::simulation.m_grid.m_columns[col];
				for(int row = 0; row < (int)column.size(); row++)
				{
					CryptBox& box = column[row];
					for(int cryptId = 0; cryptId < box.m_positions->size(); cryptId++)
					//for(int cell = 0; cell < Simulation::crypt->deadcells.size(); cell++)
					{
						//Vector3D& vec = Simulation::crypt->deadcells[cell];
						Vector2D& vec = (*box.m_positions)[cryptId];

						if(box.m_deadCrypts[cryptId] == 0 && box.m_mutated[cryptId] == 0 && box.m_mucosalMutation[cryptId] <= 20.0)
						{
							DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(vec.x, 0.0f, vec.y);
							//DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(10.0f, 10.0f, 10.0f);
							DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.25f, 0.25f, 0.25f);

							world = DirectX::XMMatrixMultiply(scale, world);
							world = DirectX::XMMatrixMultiply(world, proj);

							DirectX::XMFLOAT4X4 mat;
							DirectX::XMStoreFloat4x4(&mat, world);

							memcpy(matrixScratchBuffer + 16 * numInBatch, &world, 16 * sizeof(float));
							numInBatch++;

							if(numInBatch == batchSize)
							{
								DrawBatch(numInBatch);
								numInBatch = 0;
							} 
						}
					}
				}
			}
		//}
		DrawBatch(numInBatch);

		colour[0] = 0.0f;
		colour[1] = 1.0f;
		deviceContext->Map(psCBuffer,0, D3D11_MAP_WRITE_DISCARD, 0, &mappedColour);

		memcpy(mappedColour.pData, colour, 16);

		deviceContext->Unmap(psCBuffer, 0);
		
		numInBatch = 0;
		/*
		for(int i = 0; i< Simulation::crypts.size(); i++)
			{*/
			for(int col = 0; col < (int)::simulation.m_grid.m_columns.size(); col++)
			{
				std::vector<CryptBox>& column = ::simulation.m_grid.m_columns[col];
				for(int row = 0; row < (int)column.size(); row++)
				{
					CryptBox& box = column[row];
					for(int cryptId = 0; cryptId < box.m_positions->size(); cryptId++)
					//for(int cell = 0; cell < Simulation::crypt->deadcells.size(); cell++)
					{
						//Vector3D& vec = Simulation::crypt->deadcells[cell];
						Vector2D& vec = (*box.m_positions)[cryptId];

						if(box.m_deadCrypts[cryptId] == 0 && box.m_mutated[cryptId] == 0 && box.m_mucosalMutation[cryptId] > 20.0)
						{
							DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(vec.x, 0.0f, vec.y);
							//DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(10.0f, 10.0f, 10.0f);
							DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.25f, 0.25f, 0.25f);

							world = DirectX::XMMatrixMultiply(scale, world);
							world = DirectX::XMMatrixMultiply(world, proj);

							DirectX::XMFLOAT4X4 mat;
							DirectX::XMStoreFloat4x4(&mat, world);

							memcpy(matrixScratchBuffer + 16 * numInBatch, &world, 16 * sizeof(float));
							numInBatch++;

							if(numInBatch == batchSize)
							{
								DrawBatch(numInBatch);
								numInBatch = 0;
							} 
						}
					}
				}
			}
		//}
		DrawBatch(numInBatch);

		colour[0] = 1.0f;
		colour[1] = 0.0f;
		colour[2] = 0.0f;
		deviceContext->Map(psCBuffer,0, D3D11_MAP_WRITE_DISCARD, 0, &mappedColour);

		memcpy(mappedColour.pData, colour, 16);

		deviceContext->Unmap(psCBuffer, 0);
		
		numInBatch = 0;
		/*
		for(int i = 0; i< Simulation::crypts.size(); i++)
			{*/
			for(int col = 0; col < (int)::simulation.m_grid.m_columns.size(); col++)
			{
				std::vector<CryptBox>& column = ::simulation.m_grid.m_columns[col];
				for(int row = 0; row < (int)column.size(); row++)
				{
					CryptBox& box = column[row];
					for(int cryptId = 0; cryptId < box.m_positions->size(); cryptId++)
					//for(int cell = 0; cell < Simulation::crypt->deadcells.size(); cell++)
					{
						//Vector3D& vec = Simulation::crypt->deadcells[cell];
						Vector2D& vec = (*box.m_positions)[cryptId];

						if(box.m_deadCrypts[cryptId] == 0 && box.m_mutated[cryptId] == 1)
						{
							DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(vec.x, 0.0f, vec.y);
							//DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(10.0f, 10.0f, 10.0f);
							DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.25f, 0.25f, 0.25f);

							world = DirectX::XMMatrixMultiply(scale, world);
							world = DirectX::XMMatrixMultiply(world, proj);

							DirectX::XMFLOAT4X4 mat;
							DirectX::XMStoreFloat4x4(&mat, world);

							memcpy(matrixScratchBuffer + 16 * numInBatch, &world, 16 * sizeof(float));
							numInBatch++;

							if(numInBatch == batchSize)
							{
								DrawBatch(numInBatch);
								numInBatch = 0;
							} 
						}
					}
				}
			}
		//}
		DrawBatch(numInBatch);
	}

	void DrawSceneByCellType()
	{
		float redCol[] = {1.0f, 0.0f, 0.0f, 1.0f};
		float purpleCol[] = {0.7f, 0.0f, 1.0f, 1.0f};
		float blueCol[] = {0.0f, 0.87f, 1.0f, 1.0f};
		/*
		for(int i = 0; i< Simulation::crypts.size(); i++)
		{
			DrawCellsInBounds(Simulation::crypts[i]->m_basicG0StemBoundary, Simulation::crypts[i]->m_cryptHeight * -1.0f, purpleCol, true); // quiescent stem cells
			DrawCellsInBounds(Simulation::crypts[i]->m_basicG0StemBoundary, Simulation::crypts[i]->m_cryptHeight * -1.0f, purpleCol, false); // cycling stem cells
			DrawCellsInBounds(1000.0f, Simulation::crypts[i]->m_basicG0StemBoundary, blueCol, false); // cycling proliferating cells
			DrawCellsInBounds(Simulation::crypts[i]->m_basicG0ProliferationBoundary, Simulation::crypts[i]->m_basicG0StemBoundary, blueCol, true); // quiescent proliferating cells
			DrawCellsInBounds(1000.0, Simulation::crypts[i]->m_basicG0ProliferationBoundary, redCol, true); // differentiated cells
		}*/
	}
}