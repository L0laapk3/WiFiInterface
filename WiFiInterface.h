#ifndef WiFiInterface
#define WiFiInterface_h

class WiFiInterfaceClass {
public:
  WiFiInterfaceClass(void);

  void connect(char* APName, std::function<void()> requestUserFn = nullptr, std::function<void()> doneUserFn = nullptr);

protected:
  String getNetworkString();
};

#if !defined(NO_GLOBAL_INSTANCES)
extern WiFiInterfaceClass WiFiInterface;
#endif

#endif