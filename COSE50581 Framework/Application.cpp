#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}
	
    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize the world matrix
	XMStoreFloat4x4(&_sunWorld, XMMatrixIdentity());
	XMStoreFloat4x4(&_planet1World, XMMatrixIdentity());
	XMStoreFloat4x4(&_planet2World, XMMatrixIdentity());
	XMStoreFloat4x4(&_moon1World, XMMatrixIdentity());
	XMStoreFloat4x4(&_moon2World, XMMatrixIdentity());

    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 6.5f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, -3.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));

    // Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	/*HRESULT hr;*/
	HRESULT hr;

    // Create vertex buffer for Cube 1
    /*SimpleVertex vertices[] =
    {
        { XMFLOAT3( -1.0f, 1.0f, 0.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 0.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, 0.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 0.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
    };*/

	// Create vertex buffer for Cube 2 (Can remove)
	/*SimpleVertex vertices1[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
	};*/

	SimpleVertex vertices[] =
	{	// Top Left - v0
		{ XMFLOAT3(-1.0f, 1.0f,  -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
		// Top right - v1		 
		{ XMFLOAT3(1.0f, 1.0f,   -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		// Bottom left - v2		 
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		// Bottom right - v3	 
		{ XMFLOAT3(1.0f, -1.0f,  -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		// Top left Z=1 - v4
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		// Top right Z=1 - v5
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		// Bottom left Z=1 - v6
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		// Bottom right Z=1 - v7
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },

	};

	// D3D11 buffer for Cube 1
    /*D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;*/

	//D3D11 buffer for Cube 2 (Can remove)
	/*D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 8;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
*/

	//D3D11 buffer for Cube 3 (Can remove)
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 8;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// InitData for Cube 1
    /*D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;*/

	// InitData for Cube 2 (Can remove)
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	// hr for Cube 1
    //hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

	// hr for Cube 2 (Can remove)
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

 /*   if (FAILED(hr))
        return hr;*/

	// Cube 1 check (Can remove)
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
	WORD indices[] =
	{
		//Front
		0, 1, 2,
		2, 1, 3,
		//Left Side
		4, 0, 6,
		6, 0, 2,
		//Back
		5, 4, 6,
		5, 6, 7,
		//Right Side
		3, 1, 5,
		3, 5, 7,
		//Top
		4, 5, 1,
		4, 1, 0,
		//Bottom
		6, 3, 7,
		6, 2, 3


	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;
	
	// Define our depth/stencil buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// End of depth/stencil definition

	// Create depth/stencil buffer:

	// The first parameter is the depth/stencil description, the second parameter is the state (we don't have one so we set it to nullptr, and the third is the returned depth/stencil buffer.
	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);


	// Now we need to bind the depth to the OM stage of the pipeline

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

	// Originally the third parameter was set to nullptr because we didn't have a depth/stencil view. But now we have one!
	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

	// This struct object tells the program the type of rasterizer state we want to create.
	D3D11_RASTERIZER_DESC wfdesc;
	// This makes sure the memory is cleared
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	// D3D11_FILL_WIREFRAME for wireframe rendering, or D3D11_FILL_SOLID for solid rendering
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	// Disables culling, which means we can see the backs of the cubes as they spin.
	wfdesc.CullMode = D3D11_CULL_NONE;
	// Creates the new render state. The render state will be bound to the RS (RenderState)
	// stage of the pipeline, so we create the render state with the ID3D11Device::CreateRasterizerState()
	// method. The first parameter is the description of our render state, and the second is a pointer
	// to a ID3D11RasterizerState object which will hold our new render state.
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

	// Sometimes different object need different render states, don't set the render state at start of program,
	// set it before you render each object. Default render state = nullptr.
	_pImmediateContext->RSSetState(_wireFrame);


    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);

	InitIndexBuffer();

    // Set index buffer
    _pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
	if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();
	if (_wireFrame) _wireFrame->Release();
}

void Application::Update()

{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    //
    // Animate the cubes
    //
	// Cube 1 transformation (The Sun)
	XMStoreFloat4x4(&_sunWorld, XMMatrixScaling(0.75f, 0.75f, 0.75f) * XMMatrixRotationY(t));
	// Cube 2 transformation (Planet 1 - left)
	XMStoreFloat4x4(&_planet1World, XMMatrixScaling(0.5f, 0.5f, 0.5f) *  XMMatrixRotationY(-t) * XMMatrixTranslation(-3.00f, 0.0f, 0.0f) * XMMatrixRotationY(-t));
	// Cube 3 transformation (Planet 2 - right)
	XMStoreFloat4x4(&_planet2World, XMMatrixScaling(0.5f, 0.5f, 0.5f) *  XMMatrixRotationY(-t) * XMMatrixTranslation(3.00f, 0.0f, 0.0f) * XMMatrixRotationY(-t));
	// Cube 4 transformation (Moon 1 - left)
	XMStoreFloat4x4(&_moon1World, XMMatrixRotationY(-t) * XMMatrixTranslation(-5.00f, 0.0f, 0.0f) * XMMatrixScaling(0.25f, 0.25f, 0.25f) * XMMatrixRotationY(-t * 3) * XMMatrixTranslation(-3.00f, 0.0f, 0.0f) * XMMatrixRotationY(-t));
	// Cube 5 transformation (Moon 2 - right)
	XMStoreFloat4x4(&_moon2World, XMMatrixRotationY(-t) * XMMatrixTranslation(5.00f, 0.0f, 0.0f) * XMMatrixScaling(0.25f, 0.25f, 0.25f) * XMMatrixRotationY(-t * 3) * XMMatrixTranslation(3.00f, 0.0f, 0.0f) * XMMatrixRotationY(-t));

	for (int i = 0; i < ASTEROID_COUNT; i++)
	{

	}

	if (GetAsyncKeyState(0x57))
	{
		up = ++up;
		XMVECTOR Eye = XMVectorSet(0.0f, up, -10.0f, 0.0f);
		XMVECTOR At = XMVectorSet(0.0f, -3.0f, 0.0f, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));
		Sleep(60);
	}

	//XMStoreFloat4x4(&_world4, XMMatrixTranslation(6.5f, 0.0f, 0.0f) * XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationY(t * 1.2) * XMMatrixTranslation(9.0f, 0.0f, 0.0f) * XMMatrixRotationY(t));


}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	 /*Now we need to clear the depth/stencil view every frame, like we do with
	 the above RenderTargetView.
	 The first value is the depth/stencil view we want to clear, the second is
	 an enumerated type, ordered together, specifying the part of the depth
	 /stencil view we want to clear, and the fourth parameter is the value we
	 want to clear the depth to. We set this to 1.0f, since 1.0f is the 
	 largest depth value anything can have. This makes sure everything is drawn
	 on the screen. If it were 0.0f, nothing would be drawn since all the depth
	 values of the pixel fragments would be between 0.0f and 1.0f. The last 
	 parameter is the value we set the stencil to. We set it to 0 since we're 
	 not using it.*/

	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	_pImmediateContext->RSSetState(nullptr);
	// Load the first world (Sun) matrix to CPU
	XMMATRIX world = XMLoadFloat4x4(&_sunWorld);
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);
    //
    // Update variables
    //
    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);

	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    //
    // Renders a triangle
    //
	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);    

	_pImmediateContext->RSSetState(_wireFrame);
	// Load the second world (Planet 1) matrix to CPU
	world = XMLoadFloat4x4(&_planet1World);
	// Prime the second world matrix for passing to GPU
	cb.mWorld = XMMatrixTranspose(world);
	// Pass the second world matrix to GPU
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	// Draw the second world matrix
	_pImmediateContext->DrawIndexed(36, 0, 0);

	_pImmediateContext->RSSetState(_wireFrame);
	// Load the third world (Planet 2) matrix to CPU
	world = XMLoadFloat4x4(&_planet2World);
	// Prime the third world matrix for passing to GPU
	cb.mWorld = XMMatrixTranspose(world);
	// Pass the third world matrix to GPU
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	// Draw the third world matrix
	_pImmediateContext->DrawIndexed(36, 0, 0);

	_pImmediateContext->RSSetState(nullptr);
	// Load the fourth world (Moon 1) matrix to CPU
	world = XMLoadFloat4x4(&_moon1World);
	// Prime the fourth world matrix for passing to GPU
	cb.mWorld = XMMatrixTranspose(world);
	// Pass the fourth world matrix to GPU
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	// Draw the fourth world matrix
	_pImmediateContext->DrawIndexed(36, 0, 0);

	_pImmediateContext->RSSetState(nullptr);
	// Load the fifth world (Moon 2) matrix to CPU
	world = XMLoadFloat4x4(&_moon2World);
	// Prime the fifth world matrix for passing to GPU
	cb.mWorld = XMMatrixTranspose(world);
	// Pass the fifth world matrix to GPU
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	// Draw the fifth world matrix

	_pImmediateContext->DrawIndexed(36, 0, 0);

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}