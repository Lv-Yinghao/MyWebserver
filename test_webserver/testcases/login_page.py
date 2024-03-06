from utils import DriverUtil
from selenium.webdriver import Keys
from selenium.webdriver.common.by import By

newPageURL = "http://192.168.179.128:9006/2CGISQL.cgi"

class LoginPage(object):
    def __init__(self):
        self.driver = DriverUtil.getDriver()
        self.name = (By.NAME, 'user')
        self.password = (By.NAME, 'password')
        self.btn = (By.TAG_NAME, 'button')

    # 定位用户名方法
    def find_username(self):
        return self.driver.find_element(self.name[0], self.name[1])

    # 定位密码方法
    def find_password(self):
        return self.driver.find_element(self.password[0], self.password[1])

    # 定位登录按钮方法
    def find_login_btn(self):
        return self.driver.find_element(self.btn[0], self.btn[1])

    # 定位登录失败提示方法
    def find_login_fail_msg(self):
        return self.driver.find_element(By.XPATH, "/html/body/div[2]/div")

    # 获得用户名或密码长度不合适的提示信息
    def find_username_and_password_length_msg(self):
        return DriverUtil.getDriver().find_element(By.XPATH, "/html/body/div[2]/div")


class LoginHandle(object):
    def __init__(self):
        self.login_page = LoginPage()

    # 输入用户名方法
    def input_username(self, name):
        self.login_page.find_username().send_keys(name)

    # 输入密码方法
    def input_password(self, password):
        self.login_page.find_password().send_keys(password)

    # 点击登录按钮方法
    def click_login_btn(self):
        self.login_page.find_login_btn().click()

    # 获取登录失败提示的文本
    def get_login_fail_msg(self):
        return self.login_page.find_login_fail_msg().text

    # 获取用户名的required属性
    def get_username_required_attribute(self):
        return self.login_page.find_username().get_attribute("required")

    # 获取密码的required属性
    def get_password_required_attribute(self):
        return self.login_page.find_password().get_attribute("required")

    # 获得密码的type属性
    def get_password_type_attribute(self):
        return self.login_page.find_password().get_attribute("type")

    # 获得用户框的文本
    def get_username_text(self):
        return self.login_page.find_username().text

    # 获得密码框的文本
    def get_password_text(self):
        return self.login_page.find_username().text

    # 获取用户名或密码长度不合适的提示信息文本
    def get_judge_username_and_password_length_msg(self):
        return self.login_page.find_username_and_password_length_msg().text


class LoginTask(object):
    def __init__(self):
        self.login_handle = LoginHandle()

    # 登录方法
    def login_method(self, name, pwd):
        self.login_handle.input_username(name)
        self.login_handle.input_password(pwd)
        self.login_handle.click_login_btn()

    # 验证登录失败方法
    def judge_login_fail(self):
        return self.login_handle.get_login_fail_msg() == "提示：用户名或密码错误，请重试"

    # 验证登录成功方法
    def judge_login_succeed(self):
        return DriverUtil.getDriver().current_url == newPageURL

    # 判断用户名是否为空
    def judge_user_empty(self):
        return self.login_handle.get_username_required_attribute()

    # 判断密码是否为空
    def judge_password_empty(self):
        return self.login_handle.get_password_required_attribute()

    # 判断用户名是否过长
    def judge_username_too_long(self):
        return self.login_handle.get_judge_username_and_password_length_msg() == "用户名过长，请重新输入。"

    # 判断用户名是否过短
    def judge_username_too_short(self):
        return self.login_handle.get_judge_username_and_password_length_msg() == "用户名过短，请重新输入。"

    # 判断密码是否过长
    def judge_password_too_long(self):
        return self.login_handle.get_judge_username_and_password_length_msg() == "密码过长，请重新输入。"

    # 判断密码是否过短
    def judge_password_too_short(self):
        return self.login_handle.get_judge_username_and_password_length_msg() == "密码过短，请重新输入。"

    # 判断密码是否加密显示
    def judge_password_encryption(self):
        return self.login_handle.get_password_type_attribute() == "password"

    # 判断是否登录失败后清空输入框
    def judge_clear_inputBox(self):
        return self.login_handle.get_password_text() == "" and self.login_handle.get_username_text() == ""

    # 使用tab和enter键完成登录
    def judge_automated_login(self):
        driver = DriverUtil.getDriver()
        active_element = driver.switch_to.active_element
        active_element.send_keys(Keys.TAB)

        active_element = driver.switch_to.active_element
        active_element.send_keys("root")
        active_element.send_keys(Keys.TAB)

        active_element = driver.switch_to.active_element
        active_element.send_keys("123456")
        active_element.send_keys(Keys.TAB)

        active_element = driver.switch_to.active_element
        active_element.click()

        return self.judge_login_succeed()
