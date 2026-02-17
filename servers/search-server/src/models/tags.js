import pkg from 'sequelize';
const { DataTypes } = pkg;

export default {
	projectID: {
		type: DataTypes.INTEGER,
		allowNull: false,
		primaryKey: true,
	},
	tag: {
		type: DataTypes.STRING,
		allowNull: false,
	},
};
