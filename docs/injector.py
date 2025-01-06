import os
import sys

projectPaths = []
projectNames = []
generated_h_md = []
included_md = []

def inject(projectPaths):
    for projectPath in projectPaths:

        # Check if project path exists
        if not os.path.exists(projectPath):
            print('Project path does not exist:', projectPath)
            continue

        # Check if project path + src
        if not os.path.exists(projectPath + '/src'):
            print('Project path does not contain src directory:', projectPath)
            continue

        injectProject(projectPath)
        generated_h_md.clear()

def injectProject(projectPath):
    projectName = os.path.basename(projectPath)
    projectNames.append(projectName)

    # Create _headers directory if it doesn't exist
    if not os.path.exists(f'source/_headers/{projectName}'):
        os.makedirs(f'source/_headers/{projectName}')

    # Create _projects directory if it doesn't exist
    if not os.path.exists(f'source/_projects/{projectName}'):
        os.makedirs(f'source/_projects/{projectName}')

    # Create _static directory if it doesn't exist
    if not os.path.exists(f'source/_static/{projectName}'):
        os.makedirs(f'source/_static/{projectName}')

    injectProjectApiReference(projectName, projectPath)
    injectProjectHeaders(projectName, projectPath)
    injectProjectSpecificDocumentation(projectName, projectPath)
    injectProjectIndex(projectName, projectPath)
    injectProjectMain(projectName, projectPath)

def injectProjectApiReference(projectName, projectPath):
    print('Injecting project api-reference:', projectName)
    fn = f'source/_projects/{projectName}/' + projectName + '-api.md'
    with open(fn, 'w+') as file:
        file.write(f'''# API Reference: {projectName}
```{{doxygenindex}}
:project: doxygen-{projectName}
```''' % (locals()))

def injectProjectHeaders(projectName, projectPath):
    print('Injecting project headers:', projectName)
    generated_h_md.clear()
    
    # Find all .h files
    headers = [f for f in os.listdir(projectPath+'/src') if f.endswith('.h')]
    
    for header in headers:
        # Generate .dot files
        doth_folder = f'source/_dot/{projectName}'
        if not os.path.exists(doth_folder):
            os.makedirs(doth_folder)
        hasInc = generateDotPngInc(projectName, projectPath, header)
        hasDepInc = generateDotPngDepInc(projectName, projectPath, header)

        # Switch .h to .md
        fn = header.replace('.h', '.md')
        with open(f'source/_headers/{projectName}/' + fn, 'w+') as file:
            generated_h_md.append(fn)
            file.write(f'''# {header}\n''')
   
            if hasInc:
                file.write(f'''## Include Graph\n''')
                inc_fn = f'../../_static/{projectName}/' + header.replace('.h', '') + '-inc.png'
                file.write(f'''```{{image}} {inc_fn}
:alt: A description of the image
:align: center
```\n\n''')
                file.write(f'***\n\n')

            if hasDepInc:
                file.write(f'''## Dependency Graph\n''')
                dep_inc_fn = f'../../_static/{projectName}/' + header.replace('.h', '') + '-dep-inc.png'
                file.write(f'''```{{image}} {dep_inc_fn}
:alt: A description of the image
:align: center
```\n\n''')
                file.write(f'***\n\n')

            file.write(f'''## API\n''')
            file.write(f'''```{{doxygenfile}} {header}
:project: doxygen-{projectName}
```\n\n''')
            
#             file.write (f"```")

def generateDotPngInc(projectName, projectPath, header):
    doxygen_path = f'doxygen-{projectName}'
    dependency_fn = header.replace('_', '__').replace('.h', '_8h') + '__incl.dot'
    dependency_fn_path = doxygen_path + '/latex/' + dependency_fn
    
    # Check if dependency file exists
    if not os.path.exists(dependency_fn_path):
        print('Dependency dot file does not exist:', dependency_fn_path)
        return False
    else:
        print('Dependency dot file exists:', dependency_fn_path)
        # Copy dependcy_fn_path to source/_projects/<projectName>/<header>.dot
        with open(f'source/_dot/{projectName}/' + header.replace('.h', '') + '-inc.dot', 'w+') as file:
            with open(dependency_fn_path, 'r') as dependency_file:
                file.write(dependency_file.read())
    return True

def generateDotPngDepInc(projectName, projectPath, header):
    doxygen_path = f'doxygen-{projectName}'
    dependency_fn = header.replace('_', '__').replace('.h', '_8h') + '__dep__incl.dot'
    dependency_fn_path = doxygen_path + '/latex/' + dependency_fn
    
    # Check if dependency file exists
    if not os.path.exists(dependency_fn_path):
        print('Dependency dot file does not exist:', dependency_fn_path)
        return False
    else:
        print('Dependency dot file exists:', dependency_fn_path)
        # Copy dependcy_fn_path to source/_projects/<projectName>/<header>.dot
        with open(f'source/_dot/{projectName}/' + header.replace('.h', '') + '-dep-inc.dot', 'w+') as file:
            with open(dependency_fn_path, 'r') as dependency_file:
                file.write(dependency_file.read())
    return True

def injectProjectSpecificDocumentation(projectName, projectPath):
    print('Injecting project specific documentation:', projectName)
    docs = []
    included_md.clear()

    # Find all .md files
    if os.path.exists(projectPath + '/docs'):
        docs = [f for f in os.listdir(projectPath + '/docs') if f.endswith('.md')]
    
    # Copy files
    for doc in docs:
        with open(f'source/_projects/{projectName}/' + doc, 'w+') as file:
            with open(projectPath + '/docs/' + doc, 'r') as doc_file:
                file.write(doc_file.read())

        included_md.append(doc)

def injectProjectIndex(projectName, projectPath):
    print('Injecting project index:', projectName)
    fn = f'source/_projects/{projectName}/' + projectName + '-index.md'
    with open(fn, 'w+') as file:
        file.write(f'''# Index: {projectName}

```{{toctree}}
:caption: Contents:
:maxdepth: 2\n''' % (locals()))

        for header in generated_h_md:
            header_reference = header.replace('.md', '')
            file.write(f'''../../_headers/{projectName}/{header_reference}\n''')
        
        file.write(f'''```''')

def injectProjectMain(projectName, projectPath):
    print('Injecting project main:', projectName)
    fn = f'source/_projects/{projectName}/' + projectName + '.md'
    with open(fn, 'w+') as file:
        file.write(f'''# {projectName}
```{{toctree}}
:caption: Contents:
:maxdepth: 5\n''')

        for md in included_md:
            file.write(f'''{md}\n''')

        file.write(f'''{projectName}-index\n''')
        file.write(f'''{projectName}-api\n''')
        file.write(f'''```''')

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: python injector.py <project>')
        exit(1)
        
    # Read first argument from command line
    projectPaths = sys.argv[1:]
    inject(projectPaths)

    # Create index.md
    with open('source/index.md', 'w+') as file:
        file.write(f'''# Evert PMU Firmware Documentation
```{{toctree}}
:titlesonly:
:glob:\n''')

        for projectName in projectNames:
            file.write(f'''_projects/{projectName}/{projectName}\n''')

        file.write(f'''```''')

    print('Injection complete:', projectNames)