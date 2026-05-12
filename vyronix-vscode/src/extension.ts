import * as vscode from 'vscode';
import * as cp from 'child_process';
import * as path from 'path';

export function activate(context: vscode.ExtensionContext) {
	let outputChannel = vscode.window.createOutputChannel("VYRONIX Output");
	const diagnosticCollection = vscode.languages.createDiagnosticCollection('vyronix');

	const updateDiagnostics = (document: vscode.TextDocument) => {
		if (path.extname(document.fileName) !== '.vx') return;

		const config = vscode.workspace.getConfiguration('vyronix');
		const compilerPath = config.get<string>('compilerPath') || 'vyronixc.exe';
		const filePath = document.fileName;

		cp.exec(`"${compilerPath}" "${filePath}" -o out_tmp.vxb`, (err, stdout, stderr) => {
			diagnosticCollection.clear();
			const diagnostics: vscode.Diagnostic[] = [];

			const output = stderr || stdout;
			if (output) {
				// Pattern: [TYPE ERROR] filename:line:column: message
				const regex = /\[(SYNTAX|SEMANTIC) ERROR\] (.*?):(\d+):(\d+): (.*)/g;
				let match;
				while ((match = regex.exec(output)) !== null) {
					const line = parseInt(match[3]) - 1;
					const col = parseInt(match[4]) - 1;
					const message = match[5];
					
					const range = new vscode.Range(line, col, line, col + 1);
					const diagnostic = new vscode.Diagnostic(range, message, vscode.DiagnosticSeverity.Error);
					diagnostics.push(diagnostic);
				}
			}
			diagnosticCollection.set(document.uri, diagnostics);
		});
	};

	vscode.workspace.onDidSaveTextDocument(doc => updateDiagnostics(doc));
	vscode.workspace.onDidOpenTextDocument(doc => updateDiagnostics(doc));
	if (vscode.window.activeTextEditor) {
		updateDiagnostics(vscode.window.activeTextEditor.document);
	}

	let runCommand = vscode.commands.registerCommand('vyronix.runFile', () => {
		const editor = vscode.window.activeTextEditor;
		if (!editor) {
			vscode.window.showErrorMessage("No active editor found.");
			return;
		}

		const filePath = editor.document.fileName;
		if (path.extname(filePath) !== '.vx') {
			vscode.window.showErrorMessage("Active file is not a VYRONIX (.vx) file.");
			return;
		}

		const config = vscode.workspace.getConfiguration('vyronix');
		const compilerPath = config.get<string>('compilerPath') || 'vyronixc.exe';
		const vmPath = config.get<string>('vmPath') || 'vyronixvm.exe';
		const bytecodePath = filePath.replace('.vx', '.vxb');

		outputChannel.clear();
		outputChannel.show();
		outputChannel.appendLine(`--- Compiling ${filePath} ---`);

		// Compile
		cp.exec(`"${compilerPath}" "${filePath}" -o "${bytecodePath}"`, (err, stdout, stderr) => {
			if (err) {
				outputChannel.appendLine(`Compilation error: ${stderr || stdout}`);
				return;
			}
			if (stdout) outputChannel.append(stdout);
			
			outputChannel.appendLine(`--- Running ---`);
			
			// Run VM
			cp.exec(`"${vmPath}" "${bytecodePath}"`, (err, stdout, stderr) => {
				if (err) {
					outputChannel.appendLine(`Runtime error: ${stderr || stdout}`);
					return;
				}
				outputChannel.append(stdout);
				outputChannel.appendLine(`\r\n--- Execution Finished ---`);
			});
		});
	});

	context.subscriptions.push(runCommand);

	// Definition Provider
	const definitionProvider = vscode.languages.registerDefinitionProvider('vyronix', {
		provideDefinition(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken) {
			const range = document.getWordRangeAtPosition(position);
			const word = document.getText(range);

			if (!word) return null;

			const text = document.getText();
			// Look for fn, proc, struct, let, const declarations
			const regexes = [
				new RegExp(`fn\\s+${word}\\b`, 'g'),
				new RegExp(`proc\\s+${word}\\b`, 'g'),
				new RegExp(`struct\\s+${word}\\b`, 'g'),
				new RegExp(`let\\s+${word}\\b`, 'g'),
				new RegExp(`const\\s+${word}\\b`, 'g')
			];

			for (const regex of regexes) {
				let match;
				while ((match = regex.exec(text)) !== null) {
					const pos = document.positionAt(match.index);
					return new vscode.Location(document.uri, pos);
				}
			}

			return null;
		}
	});
	context.subscriptions.push(definitionProvider);

	// Hover Provider
	const hoverProvider = vscode.languages.registerHoverProvider('vyronix', {
		provideHover(document: vscode.TextDocument, position: vscode.Position, token: vscode.CancellationToken) {
			const range = document.getWordRangeAtPosition(position);
			const word = document.getText(range);
			if (!word) return null;

			// Basic hover info
			const text = document.getText();
			if (new RegExp(`fn\\s+${word}\\b`).test(text)) return new vscode.Hover(`Function: ${word}`);
			if (new RegExp(`proc\\s+${word}\\b`).test(text)) return new vscode.Hover(`Procedure: ${word}`);
			if (new RegExp(`struct\\s+${word}\\b`).test(text)) return new vscode.Hover(`Struct: ${word}`);
			
			return null;
		}
	});
	context.subscriptions.push(hoverProvider);
}

export function deactivate() {}
