
<template>
  <el-container>
    <div class="login">
      <el-card>
        <el-row
          style="
            justify-content: center;
            align-items: center;
            margin: 20px 0 0 0;
          "
        >
          <img src="static/logo_v.png" alt="logo" height="120" />
        </el-row>
        <el-row style="justify-content: center; align-items: center">
          <h2>登录</h2>
        </el-row>
        <el-form
          class="login-form"
          :model="model"
          :rules="rules"
          ref="form"
          @submit.native.prevent="login"
        >
          <el-form-item prop="accountname">
            <el-input
              v-model="model.accountname"
              placeholder="用户名或邮箱"
              clearable
            ></el-input>
          </el-form-item>
          <el-form-item prop="password">
            <el-input
              v-model="model.password"
              placeholder="密码"
              type="password"
              show-password
              clearable
            ></el-input>
          </el-form-item>
          <el-form-item>
            <el-button
              :loading="loading"
              class="login-button"
              type="primary"
              native-type="submit"
              block
              >登录</el-button
            >
          </el-form-item>
        </el-form>
        <el-row>
          <el-col :span="8">
            <router-link to="/Register">
              <el-link>注册</el-link>
            </router-link>
          </el-col>
          <el-col :span="8"></el-col>
          <!-- <el-col :span="8" align="right">
            <router-link to="/ForgetPassword">
              <el-link>忘记密码</el-link>
            </router-link>
          </el-col> -->
        </el-row>
      </el-card>
    </div>
  </el-container>
</template>
 
<script>
const { ElMessage } = ElementPlus;
export default {
  name: "login",
  data() {
    return {
      model: {
        accountname: "",
        password: "",
      },
      loading: false,
      rules: {
        accountname: [
          {
            required: true,
            message: "请输入用户名或邮箱",
            trigger: "blur",
          },
        ],
        password: [{ required: true, message: "请输入密码", trigger: "blur" }],
      },
    };
  },
  created() {},
  methods: {
    login() {
          let formData = {
            username: "",
            password: this.model.password,
            email: "",
          };
          if (this.model.accountname.includes("@"))
            formData.email = this.model.accountname;
          else formData.username = this.model.accountname;
          axios
            .post("/api/login", {"user_name":this.model.accountname,"password":this.model.password})
            .then((response) => {
              console.log(response)
              if (response.data.statusCode == 200) {
                ElMessage({
                  message: "登录成功",
                  type: "success",
                });
                this.$router.push("/UserSpace");
              } else {
                ElMessage({
                  message: response.data.message,
                  type: "error",
                });
              }
            })
            .catch(function (error) {
              console.log(error);
            });
        
    },
    register() {
      this.$router.push("/Register");
    },
  },
};
</script>

<style scoped>
.login {
  width: 100%;
  height: 100%;
  flex: 1;
  display: flex;
  justify-content: center;
  align-items: center;
  position: absolute;
}

.login-button {
  width: 100%;
  margin-top: 20px;
}
.login-form {
  width: 250px;
}
.forgot-password {
  margin-top: 10px;
}
</style>
<!-- <style lang="scss">
$teal: #094ab1;
.el-button--primary {
  background: $teal;
  border-color: $teal;

  &:hover,
  &.active,
  &:focus {
    background: lighten($teal, 7);
    border-color: lighten($teal, 7);
  }
}
.login .el-input__inner:hover {
  border-color: $teal;
}
.login .el-input__prefix {
  background: rgb(238, 237, 234);
  left: 0;
  height: calc(100% - 2px);
  left: 1px;
  top: 1px;
  border-radius: 3px;
  .el-input__icon {
    width: 30px;
  }
}
.login .el-input input {
  padding-left: 10px;
}
.login .el-card {
  padding-top: 0;
  padding-bottom: 30px;
}
h2 {
  font-family: "Open Sans";
  letter-spacing: 1px;
  font-family: Roboto, sans-serif;
  padding-bottom: 20px;
}
a {
  color: $teal;
  text-decoration: none;
  &:hover,
  &:active,
  &:focus {
    color: lighten($teal, 7);
  }
}
.login .el-card {
  width: 340px;
  display: flex;
  justify-content: center;
}
</style> -->