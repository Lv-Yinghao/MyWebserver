from utils import DriverUtil
from selenium.webdriver.common.by import By

class IndexPage(object):
    def __init__(self):
        self.driver = DriverUtil.getDriver()
        self.register_btn = (By.XPATH, "/html/body/form[1]/div/button")
        self.login_btn = (By.XPATH, "/html/body/form[2]/div/button")

    def find_login_btn(self):
        return self.driver.find_element(self.login_btn[0],self.login_btn[1])

    def find_register_btn(self):
        return self.driver.find_element(self.register_btn[0],self.register_btn[1])


class IndexHandle(object):
    def __init__(self):
        self.index_page = IndexPage()

    def click_login(self):
        self.index_page.find_login_btn().click()

    def click_register(self):
        self.index_page.find_register_btn().click()


class IndexTask(object):
    def __init__(self):
        self.index_handle = IndexHandle()

    def go_to_login(self):
        self.index_handle.click_login()

    def go_to_register(self):
        self.index_handle.click_register()
