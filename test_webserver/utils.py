from selenium import webdriver


class DriverUtil(object):
    __driver = None

    @classmethod
    def getDriver(cls):
        if cls.__driver is None:
            cls.__driver = webdriver.Chrome()
            # cls.__driver = webdriver.firefox()
            cls.__driver.get("http://192.168.179.128:9006/")
            cls.__driver.implicitly_wait(10)
            cls.__driver.maximize_window()
        return cls.__driver

    @classmethod
    def quitDriver(cls):
        if cls.__driver:
            cls.__driver.quit()
            cls.__driver = None
