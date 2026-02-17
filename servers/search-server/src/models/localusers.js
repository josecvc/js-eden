import pkg from 'sequelize';
const { DataTypes } = pkg;

export default {
	localuserID: {
		type: DataTypes.INTEGER,
		primaryKey: true,
		autoIncrement: true,
		allowNull: false,
	},
	emailaddress: {
		type: DataTypes.STRING,
		allowNull: false,
	},
	hashedPassword: {
		type: DataTypes.STRING,
		allowNull: false,
	},
};
